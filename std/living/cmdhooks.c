/*
 * /std/living/cmdhooks.c
 *
 * This is a subpart of /std/living.c
 *
 * All command hooks are handled here, wiz, soul, tool and spell commands.
 */

#include <cmdparse.h>
#include <login.h>
#include <macros.h>
#include <std.h>
#include <options.h>
#include <functionlist.h>

/*
 * Variables, These are only accessed from routines in this module.
 */
static private mapping com_sounds;
static string   *wiz_souls,             /* The wizard soul names */
                *soul_souls,            /* The ordinary soul names */
                *tool_souls,            /* The tool soul names */
                say_string;             /* The last message said */

/*
 * Prototypes
 */
nomask public void update_hooks();
public varargs int communicate(string str = "");
public varargs int acommunicate(string str = "");
static int my_commands(string str);
private object logger;

// #define REOPEN_SOUL_ALLOWED ([ "exec_done_editing" : WIZ_CMD_NORMAL, \
//                                "pad_done_editing"  : WIZ_CMD_NORMAL, \
//                                "load_many_delayed" : WIZ_CMD_NORMAL, \
//                                "tail_input_player" : WIZ_CMD_APPRENTICE ])
#define REOPEN_SOUL_RELOAD  "_reloaded"

/*
 * Function name: cmdhooks_reset
 * Description  : Start the command parsing. The last added action is
 *                evaluated first, so speech is checked first.
 */
static void cmdhooks_reset()
{
    update_hooks();

    

    log_info("Command hooks have been reset.");
    log_debug("this_object: " + to_string(this_object()));
    log_debug("this_interactive: " + to_string(this_interactive()));
    log_debug("this_player: " + to_string(this_player()));

    add_action(#'my_commands, "", 1);
    add_action(#'communicate, "'", 2);
    add_action(#'acommunicate, "a'", 2);

    /* Get the different race-sounds. */
    if (!sizeof(com_sounds = RACESOUND[query_race()]))
    {
        com_sounds = ([ ]);
    }
}

/*
 * Function name: communicate
 * Description  : This function is called whenever the player wants to say
 *                something using the single quote ' as alias for say.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
public varargs int communicate(string str)
{
    return ({int}) CMD_LIVE_SPEECH->say_text(str);
}

/*
 * Function name: acommunicate
 * Description  : This function is called whenever the player wants to say
 *                something using the shortcut a' as alias for asay.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
public varargs int acommunicate(string str)
{
    return ({int}) CMD_LIVE_SPEECH->asay(str);
}

/*
 * Function name: query_say_string
 * Description  : This function returns the text the player last spoke using
 *                the say command. This can only be queried with this person
 *                being the interactive party for security reasons.
 * Returns      : string - the last string the player said.
 */
public nomask string query_say_string()
{
    if (this_interactive() != this_object())
    {
        return "";
    }

    return say_string;
}

/*
 * Function name: set_say_string
 * Description  : We store the text the player says to allow guilds to modify
 *                the verb based on the text. In order not to allow people to
 *                put words in your mouth, this function may only be called
 *                from the speech soul itself.
 * Arguments    : string str - the text to say.
 */
public nomask void set_say_string(string str)
{
    if (object_name(previous_object()) == CMD_LIVE_SPEECH)
    {
        say_string = str;
    }
}

/*
 * Function name: race_sound
 * Description  : This function returns the VBFC value for the sound a
 *                particular living hears when this player speaks. It
 *                operates on previous_object(-1). Notice that we use
 *                query_race rather than query_race_name since the first
 *                will always return a true and valid race name. The
 *                person speaking is this_player().
 * Returns      : string - the race sound the receiver hears.
 */
public string race_sound()
{
    string raceto = ({string}) previous_object(-1)->query_race();

    if (!com_sounds[raceto])
    {
        return "says";
    }

    return com_sounds[raceto];
}

/*
 * Function name: actor_race_sound
 * Description  : This function returns the sound this_player() makes when
 *                he or she speaks. By default this is 'say'.
 * Returns      : string - the race sound the receiver hears.
 */
public string actor_race_sound()
{
    return "say";
}

/*
 * Function name: query_com_sounds
 * Description  : Returns the mapping with the sounds the way people
 *                understand the speech of this player.
 * Returns      : mapping - the mapping.
 */
public mapping query_com_sounds()
{
    return secure_var(com_sounds);
}

/*
 * Function name:   start_souls
 * Description:     Tell the souls that we are using them, this is used to
 *                  add sublocations for the living object. Also call
 *                  'replace_soul' so that an obsolete soul can rederict
 *                  the usage to another newer soul/souls.
 * Arguments:       souls: an array with all souls that should be started
 */
nomask public string *start_souls(string *souls)
{
    int il, rflag;
    mixed ob;
    string *replace_souls, *used_souls, *tmp;
    mapping replaced;

    used_souls = ({});
    replaced = ([]);

    log_debug("Start_souls: %O", souls);

    do
    {
        rflag = 0;
        for (replace_souls = ({}), il = 0; il < sizeof(souls); il++)
        {

            ob = souls[il];
            log_debug("Start_soul: %s", ob);
            
            catch(ob->teleledningsanka());
            ob = find_object(ob);
            if (ob)
            {
                if (replaced[ob]) /* Dont replace twice */
                    continue;
                else
                {
                    tmp = ({string *}) ob->replace_soul();
                    replaced[ob] = 1;
                }

                if (stringp(tmp))
                {
                    replace_souls += ({ (string) tmp });
                    rflag = 1;
                }
                else if (pointerp(tmp))
                {
                    replace_souls += tmp;
                    rflag = 1;
                    if (souls[il] in tmp)
                        tmp = 0;
                }

                if ((tmp == 0) && !(souls[il] in used_souls))
                {
                    ob->using_soul(this_object());
                    used_souls += ({ souls[il] });
                }
            }
            else
                used_souls += ({ souls[il] });
        }
        if (rflag)
            souls = replace_souls + ({});
    } while (rflag);

    return used_souls;
}

/*
 * Function name: query_wizsoul_list
 * Description  : Give back the array with filenames of wizard souls.
 * Returns      : string * - the wizard soul list.
 */
nomask public string *query_wizsoul_list()
{
    return secure_var(wiz_souls);
}

/*
 * Function name:   load_wiz_souls
 * Description:     Load the wizard souls into the player.
 * Returns:         True if successful.
 */
static nomask int load_wiz_souls()
{
    int rank;

    log_debug("Loading wizard souls for player: %s", to_string(this_object()));
    if (!sizeof(geteuid(this_object())))
    {
        write("PANIC! Player has no euid!\n");
        log_error("PANIC! Player %s has no euid!\n", to_string(this_object()));
        return 0;
    }

    /* Only wizards can have wizard souls. */

    
    log_debug("Checking wizard rank for euid: %s", geteuid(this_object()));
    log_debug("SECURITY: %s", SECURITY);
    log_debug("Function: %s", to_string(function_exists("query_wiz_rank", FEXISTS_LINENO)));
//    if (rank = ({int}) SECURITY->query_wiz_rank(geteuid(this_object())))
    if (rank = WIZ_KEEPER)
    {
        log_debug("Wizard rank found: %d", rank);
        wiz_souls = ({string *}) WIZ_SOUL(rank)->get_soul_list();
        wiz_souls -= ({MBS_SOUL});
    }
    else
    {
        wiz_souls = ({ });
        log_debug("No wizard rank found for euid: %s", geteuid(this_object()));
        return 1;
    }

    if (!sizeof(wiz_souls))
    {
        write("Error loading wizard soul list. No wizard soul loaded.\n");
        log_error("Error loading wizard soul list. No wizard soul loaded.\n");
        return 0;
    }

    wiz_souls = start_souls(wiz_souls);
    log_debug("Successfully loaded wizard souls for player: %s", to_string(this_object()));
    return 1;
}

/*
 * Function name: load_command_souls
 * Description  : Load the command souls into the player.
 * Returns      : int 1/0 - success/failure.
 */
nomask public int load_command_souls()
{
    log_debug("Loading command souls for player: %s", to_string(this_object()));
    
    soul_souls = query_cmdsoul_list();
    if (!sizeof(soul_souls))
    {
        soul_souls = NPC_SOULS;
    }

    soul_souls = start_souls(soul_souls);
    update_cmdsoul_list(soul_souls);
    log_debug("Successfully loaded command souls for player: %s", to_string(this_object()));
    return 1;
}

/*
 * Function name:   load_tool_souls
 * Description:     Load the tool souls into the player.
 * Returns:         True upon success.
 */
nomask public int load_tool_souls()
{
    log_debug("Loading tool souls for player: %s", to_string(this_object()));
    if ((({int}) SECURITY->query_wiz_rank(geteuid()) < WIZ_NORMAL) ||
        !interactive(this_object()))
    {
        tool_souls = ({});
        return 0;
    }

    tool_souls = query_tool_list();
    if (!sizeof(tool_souls))
    {
        /* This must be this_object()-> so don't touch! */
        this_object()->add_toolsoul(TRACER_TOOL_SOUL);
        tool_souls = query_tool_list();
    }

    tool_souls = start_souls(tool_souls);
    update_tool_list(tool_souls);
    log_debug("Successfully loaded tool souls for player: %s", to_string(this_object()));
    return 1;
}

/*
 * Function name:   my_commands
 * Description:     Try to find and perform a command.
 * Arguments:       str - the argument string.
 * Returns:         True if the command was found.
 */
static int my_commands(string str)
{
    int    i, rv;
    object ob;
    string verb = query_verb();
    int    size;

    /* Don't waste the wiz-souls and toolsouls on mortals.
     */
//    if (query_wiz_level())
    if (1)
        {
        /* This construct with while is faster than any for-loop, so keep
         * it this way.
         */
        size = sizeof(wiz_souls);
        i = -1;

        while(++i < size)
        {
            ob = find_object(wiz_souls[i]);
            if (!ob)
            {
                if (catch(wiz_souls[i]->teleledningsanka()))
                    tell_object(this_object(),
                        "Yikes, baaad soul: " + wiz_souls[i] + "\n");

                ob = find_object(wiz_souls[i]);

                if (!ob)
                    continue;
            }

            if (({int}) ob->exist_command(verb))
            {
                ob->open_soul(0);
//                export_uid(ob);
                ob->open_soul(1);
                rv = ({int}) ob->do_command(verb, str);
                ob->open_soul(0);

                if (({int}) SECURITY->query_restrict(query_real_name()) & RESTRICT_LOG_COMMANDS)
                    SECURITY->log_restrict(verb, str);

                if (rv)
                    return 1;
            }
        }

        size = sizeof(tool_souls);
        i = -1;

        while(++i < size)
        {
            ob = find_object(tool_souls[i]);
            if (!ob)
            {
                if (catch(tool_souls[i]->teleledningsanka()))
                    tell_object(this_object(),
                        "Yikes, baaad soul: " + tool_souls[i] + "\n");

                ob = find_object(tool_souls[i]);

                if (!ob)
                    continue;
            }

            if (({int}) ob->exist_command(verb))
            {
                log_debug("Found command soul: %s", ob);
                log_debug("Command: %s", verb);
                log_debug("String: %s", str);
                ob->open_soul(0);
//                export_uid(ob);
                ob->open_soul(1);
                rv = ({int}) ob->do_command(verb, str);
                ob->open_soul(0);

                if (({int}) SECURITY->query_restrict(query_real_name()) & RESTRICT_LOG_COMMANDS)
                    SECURITY->log_restrict(verb, str);

                if (rv)
                    return 1;
            }
        }
    }

    size = sizeof(soul_souls);
    i = -1;
    while(++i < size)
    {
        ob = find_object(soul_souls[i]);

        if (!ob)
        {
            if (catch(soul_souls[i]->teleledningsanka()))
                tell_object(this_object(),
                    "Yikes, baaad soul: " + soul_souls[i] + "\n");

            ob = find_object(soul_souls[i]);

            if (!ob)
                continue;
        }

        if (({int}) ob->exist_command(verb))
        {
            if (({int}) ob->do_command(verb, str))
                return 1;
        }
    }

    /* Allow npcs to cast spells using the spell name as a verb. */
    if (query_npc() &&
        (ob = ({object}) this_object()->find_spell(verb)))
    {
        this_object()->start_spell(verb, str, ob);
        return 1;
    }

    return 0;
}

/*
 * Function name: reopen_soul
 * Description  : This function allows for the euid of this player to be
 *                re-exported in only a very limited number of cases.
 */
// nomask public void reopen_soul()
// {
//     object ob  = previous_object();
//     string fun = calling_function();

//     /* Check carefully. */
//     if ((!sizeof(REOPEN_SOUL_ALLOWED[fun])) ||
//         (object_name(ob) != REOPEN_SOUL_ALLOWED[fun]) ||
//         (!interactive(this_object())))
//     {
//         return;
//     }

//     ob->open_soul(0);
//     export_uid(ob);
//     ob->open_soul(1);
//     call_other(ob, (fun + REOPEN_SOUL_RELOAD));
//     ob->open_soul(0);
// }

/*
 * Function name: update_hooks
 * Description  : This function loads and initializes all wizards souls,
 *                tool souls and command souls the player can have.
 */
nomask public void update_hooks()
{
    load_wiz_souls();
    load_tool_souls();
    load_command_souls();
}

/*
 * Function name:   cmdhooks_break_spell
 * Description:     Break the preparation for a spell. Note that the caster
 *                  still suffers the attack delay.
 * Returns:         True if a spell was being prepared
 */
varargs public int cmdhooks_break_spell(string msg)
{
    /* Functionality moved into spells.c */
    return ({int}) this_object()->break_spell(msg);
}
