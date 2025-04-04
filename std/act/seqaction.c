/*
  /std/seqaction.c

  This is the first attempt at intelligent npc's. The basic idea was
  from Zellski's command/function sequences.


  Typical usage:

          seq_new("talk"); seq_new("walk");

          seq_addfirst("talk", command_sequence1);
          seq_addfirst("walk", command_sequence2);

  Typical command sequence:

  ({
      "smile", 3, "say How are you doing?", "north", "@@find_dwarf"
  })

  Integers are delayvalues in heart_beats time the slow factor.

  The slow factor is set so that the game does not get overwhelmed with
  monster sequences.

  Observe the "@@" constructs. This is the way to make truly intelligent
  behaviour. Use the VBFC just as usual. Note also that effuserid will be 0
  in the call to these VBFC functions (as it normally is).

*/

#pragma strict_types

#include <macros.h>
#include <seqaction.h>
#include <configuration.h>

static  mixed   *seq_commands;          /* Array of arrays holding actions
                                           to do each heart_beat */
static  string  *seq_names;             /* id of a sequence */
static  int     *seq_flags;             /* flags of a sequence */
static  int     seq_active,
                *seq_cpos;              /* Current position in array */
static  int     seq_delay;              /* Counter for delayed execution */
static  int     seq_next_step;          /* Steps to take in next heart_beat */

public void seq_restart();
private int seq_process_commands(int steps);

/*
 *  Description: Called from living to initialize
 */
public void
seq_reset()
{
    seq_commands = ({});
    seq_names = ({});
    seq_flags = ({});
    seq_cpos = ({});
    seq_active = 0;
    seq_delay = 0;
    seq_next_step = 0;
    configure_object(this_object(), OC_HEART_BEAT, 0);
}

/*
 * The heart_beat function is called by the driver every 2 seconds
 * if heart_beat is enabled for this object
 */
public void heart_beat()
{
    int stopseq, stepped;
    
    stopseq = ((time() - ({int}) this_object()->query_last_met_interactive()) > SEQ_STAY_AWAKE);
    
    if (stopseq && !seq_active)
        return;
        
    // Handle delayed execution
    if (seq_delay > 0) {
        seq_delay--;
        return;
    }
    
    // Process commands
    if (seq_next_step == 0)
        seq_next_step = 1;
        
    stepped = seq_process_commands(seq_next_step);
    
    // Check if we should disable heart_beat
    if (stopseq) {
        seq_active = 0;
        configure_object(this_object(), OC_HEART_BEAT, 0);
        this_object()->add_notify_meet_interactive("seq_restart");
    }
    
    // Set up a delay if needed
    if (stepped > 1) {
        // Convert old delay to heart_beat delay
        // Original: call_out between 7.5s and 22.5s for step=1
        // Now: heart_beat every 2s
        // So divide by 2 to get approximately same timing
        seq_delay = to_int(stepped * (SEQ_SLOW / 4.0));
        seq_next_step = stepped;
    } else {
        seq_next_step = 0;
    }
}

/*
 *   Description: Processes command sequences
 *   Returns: The next delay step or 0 if no delay
 */
private int seq_process_commands(int steps)
{
    int il, newstep, stopseq, stopped;
    mixed cmd;
    mixed cmdres;

    stopseq = ((time() - ({int}) this_object()->query_last_met_interactive()) > SEQ_STAY_AWAKE);

    newstep = 0;
    stopped = 0;
    if (!steps)
        steps = 1;

    if (stopseq)
        stopped = 1;

    for (il = 0; il < sizeof(seq_names); il++)
    {
        if (seq_cpos[il] < sizeof(seq_commands[il]))
        {
            if (!stopseq || (seq_flags[il] & SEQ_F_NONSTOP) || !stopped)
                stopped = 0;

            cmd = seq_commands[il][seq_cpos[il]];
            seq_cpos[il]++;

            if (stringp(cmd) || closurep(cmd))
            {
                log_debug("Running command: %O", cmd);
                cmdres = ({mixed}) this_object()->check_call(cmd);
            }
            else if (intp(cmd))
            {
                cmdres = cmd - steps;
            }
            else
            {
                newstep = 1;
                continue;
            }

            log_debug("Running command: %O", cmdres);

            if (stringp(cmdres))
            {
                log_debug("Command result: %s", cmdres);
                command(cmdres);
                newstep = 1;
            }
            else if (intp(cmdres) && cmdres > 0)
            {
                log_debug("Command result: %d", cmdres);
                newstep = ((newstep) && (cmdres > newstep) ? newstep : cmdres);
                seq_cpos[il]--;
                seq_commands[il][seq_cpos[il]] = cmdres;
            }
            else
            {
                newstep = 1;
            }
        }
        else
        {
            seq_cpos[il] = 0;
            seq_commands[il] = ({});
        }
    }

    return newstep;
}

/*
 * Called when the living encounters an interactive player
 * and sequences has been stopped
 */
public void
seq_restart()
{
    seq_active = 1;
    seq_delay = 0;
    seq_next_step = 1;
    configure_object(this_object(), OC_HEART_BEAT, 1);
    this_object()->remove_notify_meet_interactive("seq_restart");
}

/*
 *  Description: New command sequence. Command sequences are independant
 *               named streams of commands. This function creates a stream.
 */
public varargs int
seq_new(string name, int flags)
{
    if (!IS_CLONE)
        return 0;

    if (member(seq_names, name) >= 0)
        return 0;

    if (sizeof(seq_names) >= SEQ_MAX)
        return 0;

    seq_names += ({ name });
    seq_commands += ({ ({}) });
    seq_cpos += ({ 0 });
    seq_flags += ({ flags });
    return 1;
}

/*
 *   Description: Delete an entire sequence.
 */
void
seq_delete(string name)
{
    int pos;

    if ((pos = member(seq_names, name)) < 0)
        return 0;

    seq_names[pos..pos] = ({});
    seq_commands[pos..pos] = ({});
    seq_cpos[pos..pos] = ({});
    seq_flags[pos..pos] = ({});
}

/*
 *  Description: Add a command or an array of commands first in a sequence.
 *               Note that a command can be VBFC. This enables calls to
 *               functions that returns the actual command. This function
 *               can of course do all sorts of things. If 'cmd' is a number
 *               it is interpreted as a delayvalue (in heartbeats) until
 *               the next command is issued.
 *
 *               The sequence 'name' must be created with seq_new(name)
 *               prior to the call of this function.
 */
int
seq_addfirst(string name, mixed cmd)
{
    int pos;

    if ((pos = member(seq_names, name)) < 0)
    {
        return 0;
    }

    if (!seq_active)
    {
        seq_restart();
    }

    if (!pointerp(cmd))
    {
        cmd = ({ cmd });
    }

    seq_commands[pos] = cmd +
        seq_commands[pos][seq_cpos[pos]..sizeof(seq_commands[pos])];
    seq_cpos[pos] = 0;

    return 1;
}

/*
 *  Description: Add a command or an array of commands last in a sequence.
 *               Same as seq_addfirst except the command(s) are added last
 *               in the sequence. (see seq_addfirst)
 */
int
seq_addlast(string name, mixed cmd)
{
    int pos;

    if ((pos = member(seq_names, name)) < 0)
    {
        return 0;
    }

    if (!seq_active)
    {
        seq_restart();
    }

    if (!pointerp(cmd))
    {
        cmd = ({ cmd });
    }

    seq_commands[pos] = seq_commands[pos] + cmd;

    return 1;
}

/*
 *  Description: Returns the sequence of commands for a given sequence.
 *
 *               For a specific sequence 'name' it returns the remaining
 *               commands or an empty array. If no sequence 'name' exists
 *               then 0 is returned.
 */
string *
seq_query(string name)
{
    int pos;

    if ((pos = member(seq_names, name)) < 0)
        return 0;

    return seq_commands[pos][seq_cpos[pos]..];
}

/*
 *  Description: Returns the flags for a given sequence.
 *
 */
int
seq_query_flags(string name)
{
    int pos;

    if ((pos = member(seq_names, name)) < 0)
        return 0;

    return seq_flags[pos];
}

/*
 *  Description: Returns the list of sequences (their names)
 */
string *
seq_query_names() { return seq_names + ({}); }

/*
 *  Description: Clears a given sequence from commands.
 */
void
seq_clear(string name)
{
    int pos;

    if ((pos = member(seq_names, name)) < 0)
        return;

    seq_commands[pos] = ({});
    seq_cpos[pos] = 0;
}
