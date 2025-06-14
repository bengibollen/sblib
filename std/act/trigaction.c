/*
  /std/trigaction.c

  This package handles triggering of actions due to text encountered
  by the NPC.

  Observe that this includes text that is written to the NPC, ie not
  only tell_object() but also write(). This includes among other things
  room descriptions.

  NOTICE: Triggers are obsolete. Use hooks as much as possible! For emotes,
          use emote_hook() and emote_hook_onlooker().

  Typical usage:

  trig_new("pattern","function");

  "pattern"
  		A 'parse_command()' pattern. Can be VBFC.

  "function"
  		Function in this object to call when pattern matched the
		catched text. Can be VBFC.

  Example:
                trig_new("%s 'says:' %s", "say_something");
   	          -- Will on trig call: say_something(str1, str2);

  	        trig_new("%l 'drops' %i", "drop_something");
   	          -- Will on trig call: drop_something(live1, item1);

  NOTE1:
  	There is a maximum limit of 10 '%' in a pattern.

  NOTE2:
        There can be any number of patterns (max 1000). When a match
	is made the search terminates. A match is considered made if:

	     - The pattern matches
	     - The function called returns 1         <= NOTE

         A pattern can also be considered matched if the "function" value
	 is 1 and not a string as it would normally be. This means that
	 if the "function" is VBFC and returns 1 then it is considered a
	 match.

	 Note the difference between the returned value from the
	 function called and the value of (a possible) VBFC call. The
	 VBFC value is used as the name of the function to call. BUT if
	 it does not return such a name and returns 1 instead, then it
	 is a match. (Jemand Comprende ?!?)

  NOTE3:
         If the function is VBFC then the argument returned from the
	 matched parse_command can be reached through the function:

	 	- trig_query_args()
    			- Which returns an array of the right number
			  of arguments.

	 The actual text catched can be got through the function:

		- trig_query_text()
*/


#pragma strict_types

#define MAX_TRIG_VAR 10

static 	string	*trig_patterns,		/* Patterns that trig actions */
                *trig_functions;        /* Commands to execute */
static  object  *trig_oblist;           /* List of %l / %i objects */
static  int     num_arg;                /* Number of arguments */
static	mixed 	a1, a2, a3, a4, a5,
   		a6, a7, a8, a9, a10;	/* Arguments */
static	string	cur_text;		/* Text currently catched */

mixed trig_check(string str, string pat, string func);


/*
 * Function name: catch_tell
 * Description:   This is the text that normal players gets written to
 *                their sockets.
 */
void catch_tell(string str)
{
    int il;
    string pattern, func, euid;

    log_debug("Function: catch_tell called in trigaction.c");

    if (interactive(this_object())) // Monster is possessed
    {
        write(str);
        return;
    }

    if (!sizeof(trig_patterns))
    	return;

    cur_text = str;

    for (il = 0; il < sizeof(trig_patterns); il++)
    {
        if (stringp(trig_patterns[il]))
        {
            pattern = process_string(trig_patterns[il]);
            if (trig_check(str, pattern, trig_functions[il]))
            return;
        }
    }
}


/*
 * Description: Query for current arguments returned from parse_command
 */
mixed trig_query_args()
{
    mixed arr;

    arr = ({ a1, a2, a3, a4, a5, a6, a7, a8, a9, a10 });

    return arr[..<1];
}


/*
 * Description: Query for current catched text
 */
string trig_query_text()
{
    return cur_text;
}


mixed trig_check(string str, string pat, string func)
{
    int pmatch;
    string *split, euid;
    mixed ob;

    if (!stringp(pat) || !stringp(func))
    	return 0;

    split = explode("dummy" + pat + "dummy", "%");

    if ((sizeof(split) - 1) > MAX_TRIG_VAR)
    {
    	return 0; /* Illegal pattern */
    }

    if (sizeof(trig_oblist))
    	ob = trig_oblist;
    else
	    ob = environment(this_object());

    if (!ob)
    	return;

    switch (sizeof(split) - 1)
    {
        case 1:
            pmatch = parse_command(str, ob, pat, a1);
            break;
        case 2:
            pmatch = parse_command(str, ob, pat, a1,a2);
            break;
        case 3:
            pmatch = parse_command(str, ob, pat, a1,a2,a3);
            break;
        case 4:
            pmatch = parse_command(str, ob, pat, a1,a2,a3,a4);
            break;
        case 5:
            pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5);
            break;
        case 6:
            pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5,a6);
            break;
        case 7:
            pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5,a6,a7);
            break;
        case 8:
            pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5,a6,a7,a8);
            break;
        case 9:
            pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5,a6,a7,a8,a9);
            break;
        case 10:
            pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
            break;
    }

    if (!pmatch)
    	return 0;

    num_arg = sizeof(split) - 1;

    func = process_string(func);

    if (!stringp(func))
	    return func;

    switch (sizeof(split) - 1)
    {
        case 1:
            return ({mixed}) call_other(this_object(), func, a1);
        case 2:
            return ({mixed}) call_other(this_object(), func, a1,a2);
        case 3:
            return ({mixed}) call_other(this_object(), func, a1,a2,a3);
        case 4:
            return ({mixed}) call_other(this_object(), func, a1,a2,a3,a4);
        case 5:
            return ({mixed}) call_other(this_object(), func, a1,a2,a3,a4,a5);
        case 6:
            return ({mixed}) call_other(this_object(), func, a1,a2,a3,a4,a5,a6);
        case 7:
            return ({mixed}) call_other(this_object(), func, a1,a2,a3,a4,a5,a6,a7);
        case 8:
            return ({mixed}) call_other(this_object(), func, a1,a2,a3,a4,a5,a6,a7,a8);
        case 9:
            return ({mixed}) call_other(this_object(), func, a1,a2,a3,a4,a5,a6,a7,a8,a9);
        case 10:
            return ({mixed}) call_other(this_object(), func, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
            break;
    }
    return 1;
}


/*
 * Description: Add a new pattern to trig on
 */
void trig_new(string pat, string func)
{
    int pos;

    this_object()->set_tell_active(1); /* We want all messages sent to us */

    if ((pos = member(trig_patterns, pat)) >= 0)
    	trig_functions[pos] = func;

    if (!sizeof(trig_patterns))
    {
        trig_patterns = ({});
        trig_functions = ({});
    }

    trig_patterns += ({ pat });
    trig_functions += ({ func });
}


/*
 *  Description: Delete a pattern from the ones to trig on
 */
void trig_delete(string pat)
{
    int pos;

    if ((pos = member(trig_patterns, pat)) >= 0)
    {
        trig_patterns[pos..pos] = ({});
        trig_functions[pos..pos] = ({});
    }
}


/*
 *  Description: Set the objects to trig on when pattern includes %i/%l
 */
void trig_setobjects(object *obs)
{
    trig_oblist = obs;
}

#if 0
/*
 * Function name: emote_hook
 * Description  : Whenever an emotion is performed on this living, or when it
 *                is performed on in the room in general, this function is
 *                called to let the living know about the emotion. This way we
 *                can avoid the usage of all those costly triggers.
 * Arguments    : string emote - the name of the emotion performed. This
 *                    always is the command the player typed, query_verb().
 *                object actor - the actor of the emotion.
 *                string adverbs - the adverb used with the emotion, if there
 *                    was one. When an adverb is possible with the emotion,
 *                    this argument is either "" or it will contain the used
 *                    emotion, preceded by a " " (space). This way you can
 *                    use the adverb in your reaction if you please without
 *                    having to parse it further.
 *                object *oblist - the targets of the emotion, if any.
 *                int cmd_attr - the command attributes, if any.
 *                int target - if true, this object was a target of the emote.
 */
public void
emote_hook(string emote, object actor, string adverb, object *oblist,
    int cmd_attr, int target)
{
}

/*
 * Function name: emote_hook_actor
 * Description  : Whenever a living performs an emote on others, this hook is
 *                called. This way we can avoid the usage of all those costly
 *                triggers.
 * Arguments    : string emote - the name of the emotion performed. This
 *                    always is the command the player typed, query_verb().
 *                object *oblist - the targets of the emotion.
 */
public void
emote_hook_actor(string emote, object *oblist)
{
}

/*
 * Function name: emote_hook_onlooker
 * Description  : Whenever this living sees an emotion being performed on
 *                someone else, this function is called to let the living know
 *                about the emotion. This way we can avoid the usage of all
 *                those costly triggers.
 * Arguments    : string emote - the name of the emotion performed. This
 *                    always is the command the player typed, query_verb().
 *                object actor - the actor of the emotion.
 *                string adverbs - the adverb used with the emotion, if there
 *                    was one. When an adverb is possible with the emotion,
 *                    this argument is either "" or it will contain the used
 *                    emotion, preceded by a " " (space). This way you can
 *                    use the adverb in your reaction if you please without
 *                    having to parse it further.
 *                object *oblist - the targets of the emotion.
 *                int cmd_attr - the command attributes.
 */
public void
emote_hook_onlooker(string emote, object actor, string adverb, object *oblist,
    int cmd_attr)
{
}
#endif
