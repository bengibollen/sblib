/*
 * /cmd/live/social.c
 *
 * General commands for 'nonemotive social' behaviour. And yes, killing each
 * other is a social activity, is it not? The following commands are defined:
 *
 * - aggressive
 * - assist
 * - emote
 * - forget
 * - introduce
 * - introduced
 * - invite
 * - join
 * - kill
 * - last
 * - leave
 * - mwho
 * - present
 * - remember
 * - remembered
 * - spar
 * - stop
 * - team
 * - who
 */

#pragma no_clone
#pragma no_inherit
#pragma strict_types

inherit "/std/command_driver";

#include <cmdparse.h>
#include <composite.h>
#include <const.h>
#include <libfiles.h>
#include <filter_funs.h>
#include <flags.h>
#include <formulas.h>
#include <language.h>
#include <macros.h>
#include <mail.h>
#include <options.h>
#include <std.h>
#include <stdproperties.h>
#include <libtime.h>
#include <configuration.h>
#include <interactive_info.h>

nomask int sort_name(object a, object b);
varargs int team(string str);

/*
 * Function name: smart_capitalize_name
 * Description  : Capitalizes a name. If the name starts with '(' or '[',
 *                it preserves the leading character and capitalizes the
 *                character immediately following it.
 * Arguments    : name_str - The name string to capitalize.
 * Returns      : The capitalized name string.
 */
static string smart_capitalize_name(string name_str) {
    if (!stringp(name_str) || !sizeof(name_str)) {
        return name_str;
    }

    // If the name starts with ( or [, return that char + capitalized rest.
    // This handles cases like "(name)" -> "(Name)" and "[name]" -> "[Name]".
    // It also correctly handles "(name" -> "(Name" or even just "(" -> "(".
    if (name_str[0] in "([") {
        return name_str[..0] + capitalize(name_str[1..]);
    }

    // Otherwise, just capitalize the whole string.
    return capitalize(name_str);
}

/*
 * Function name: create
 * Description  : This function is called the moment this object is created
 *                and loaded into memory.
 */
void create()
{
    configure_object(this_object(), OC_EUID, getuid());
}


/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
string get_soul_id()
{
    return "social";
}


/* **************************************************************************
 * This is a command soul.
 */
int query_cmd_soul()
{
    return 1;
}


/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
mapping query_cmdlist()
{
    return ([
             "aggressive":"aggressive",
             "assist":"assist",

             "emote":"emote",

             "forget":"forget",

             "introduce":"intro_live",
             "introduced":"introduced_list",
             "invite":"invite",

             "join":"join",

             "kill":"kill",

             "last":"last",
             "leave":"leave",

             "mwho":"who",

             "present":"intro_live",

             "remember":"remember_live",
             "remembered":"remember_live",

             "spar":"spar",
             "stop":"stop",

             "team":"team",

             "who":"who"
             ]);
}


/*
 * Function name: using_soul
 * Description:   Called once by the living object using this soul. Adds
 *                sublocations responsible for extra descriptions of the
 *                living object.
 */
public void using_soul(object live)
{
}


/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the
 * same order as in the function name list.
 * **************************************************************************/

/*
 * Aggressive - Set how aggressive we are
 *
 * This command is used to state when to fight and when to flee.
 *
 * ** not ready yet, needs fixes in the combat system **
 */
int aggressive(string str)
{
    write("Yes you are, very!\n");
    return 1;
}


/*
 * assist - Help a friend to kill someone else
 */
int assist(string str)
{
    object *obs;
    object friend;
    object victim;
    int    index;
    mixed  property_value; // Renamed from tmp

    if (!CAN_SEE_IN_ROOM(this_player()))
    {
        notify_fail("You can't see anything here.\n");
        return 0;
    }

    if (({int}) this_player()->query_ghost())
    {
        notify_fail("Umm yes, killed. That's what you are.\n");
        return 0;
    }

    if (!stringp(str))
    {
        if (!sizeof(obs = ({object *}) this_player()->query_team_others()))
        {
            notify_fail("Assist whom? You are not in a team.\n");
            return 0;
        }

        obs = ({ ({object}) this_player()->query_leader() }) - ({ 0 }) + obs;

        for (index = 0; index < sizeof(obs); index++)
        {
            if ((environment(this_player()) == environment(obs[index])) &&
                (objectp(victim = ({object}) obs[index]->query_attack())))
            {
                friend = obs[index];
                break;
            }
        }

        if (!objectp(friend))
        {
            notify_fail("None of your team members are in combat.\n");
            return 0;
        }
    }
    else
    {
        obs = parse_this(str, "[the] %l");

        if (sizeof(obs) > 1)
        {
            notify_fail("Be specific, you can't assist " +
                COMPOSITE_ALL_LIVE(obs) + " at the same time." + "\n");
            return 0;
        }
        else if (sizeof(obs) == 0)
        {
            notify_fail("Assist whom?\n");
            return 0;
        }

        friend = obs[0];
    }

    if (friend == this_player())
    {
        write("Sure! Assist yourself!\n");
        return 1;
    }

    victim = ({object}) friend->query_attack();
    if (!objectp(victim))
    {
        write(({string}) friend->query_The_name(this_player()) +
            " is not fighting anyone.\n");
        return 1;
    }

    if ((friend in ({object *}) this_player()->query_enemy(-1)) ||
        (victim == this_player()))
    {
        write("Help " + ({string}) friend->query_the_name(this_player()) +
            " to kill you? There are easier ways to commit seppuku!" +
            "\n");
        return 1;
    }

    if (environment(victim) != environment(this_player()))
    {
        notify_fail("The main target of " +
            ({string}) friend->query_the_name(this_player()) + " is not in this room.\n");
        return 0;
    }

    if (victim in ({object *}) this_player()->query_team_others())
    {
        notify_fail("But " + ({string}) victim->query_the_name(this_player()) +
            " is a team with you.\n");
        return 0;
    }

    if (({object}) this_player()->query_attack() == victim)
    {
        write("You are already fighting " +
            ({string}) victim->query_the_name(this_player()) + ".\n");
        return 1;
    }

    if (property_value = ({mixed}) environment(this_player())->query_prop(ROOM_M_NO_ATTACK))
    {
        if (stringp(property_value))
            write(property_value);
        else
            write("You sense a divine force preventing your attack.\n");
        return 1;
    }

    if (property_value = ({mixed}) victim->query_prop(OBJ_M_NO_ATTACK))
    {
        if (stringp(property_value))
            write(property_value);
        else
            write("You feel a divine force protecting this being, your " +
                "attack fails.\n");
        return 1;
    }

    if ((!({int}) this_player()->query_npc()) &&
        (({int}) this_player()->query_met(victim)) &&
        (({object}) this_player()->query_prop(LIVE_O_LAST_KILL) != victim))
    {
        this_player()->add_prop(LIVE_O_LAST_KILL, victim);
        write("Attack " + ({string}) victim->query_the_name(this_player()) +
            "?!? Please confirm by trying again.\n");
        return 1;
    }

    this_player()->reveal_me(1);

    /*
     * Check if we dare!
     */
    if (!F_DARE_ATTACK(this_player(), victim))
    {
        write("Umm... no! You do not have enough self-discipline to dare!\n");
        say(QCTNAME(this_player()) + " considers attacking " + QTNAME(victim) +
            ", though does not dare to do so.\n",
            ({ victim, this_player() }) );
        tell_object(victim, ({string}) this_player()->query_The_name(victim) +
            " looks at you as if ready to attack, though you see fear in " +
            ({string}) this_player()->query_possessive() + " eyes.\n");
        return 1;
    }

    say(QCTNAME(this_player()) + " assists " + QTNAME(friend) +
        " and attacks " + QTNAME(victim) + ".\n",
        ({ this_player(), friend, victim }) );
    tell_object(victim, ({string}) this_player()->query_The_name(victim) +
        " attacks you!\n");
    tell_object(friend, ({string}) this_player()->query_The_name(friend) +
        " assists you and attacks " + ({string}) victim->query_the_name(friend) + ".\n");
    write("You assist " + ({string}) friend->query_the_name(this_player()) +
        " and attack " + ({string}) victim->query_the_name(this_player()) + ".\n");

    this_player()->attack_object(victim);
    this_player()->add_prop(LIVE_O_LAST_KILL, victim);

    return 1;
}


/*
 * emote - Put here so NPC:s can emote (  No error messages if they do wrong,
 *         why waste cpu on NPC:s ? ;-)   )
 */
int emote(string str)
{
    if (!sizeof(str) ||
        !({int}) this_player()->query_npc())
    {
        return 0;
    }

    /* Allow for "emote 's head..." -> "The monkey's head..." and yes ''' looks
     * funny, but that really is the syntax to get a single quote in int-form.
     */
    if (str[0] != ''')
    {
        str = " " + str;
    }

    say( ({ METNAME + str + "\n", TART_NONMETNAME + str + "\n",
            UNSEEN_NAME + str + "\n" }) );

    return 1;
}


/*
 * forget - Forget someone we have remembered
 */
int forget(string name)
{
    object ob;
    int    full;

    if (!stringp(name))
    {
        notify_fail("Forget whom?\n");
        return 0;
    }

    name = lower_case(name);
    if (sscanf(name, "all about %s", name) == 1)
    {
        full = 1;
    }

    ob = find_living(name);
    if (objectp(ob) &&
        ({int}) ob->query_prop(LIVE_I_NON_FORGET))
    {
        notify_fail("It seems impossible to forget " +
            ({string}) ob->query_objective() + "!\n");
        return 0;
    }

    if (!({int}) this_player()->remove_remembered(name))
    {
        notify_fail("You do not know any " + capitalize(name) + ".\n");
        return 0;
    }

    if (!full)
    {
        this_player()->add_introduced(name);
    }
    write("Ok.\n");
    return 1;
}


/*
 * introduce - Present yourself or someone else.
 */
int intro_live(string str)
{
    string  intro_who;
    string  intro_to = "";
    object  introducee;
    object *livings;
    object *all_targets, *vis_targets;
    int     intro_self = 0;
    int     index;
    int     size;

    notify_fail(capitalize(query_verb()) + " who [to whom]?\n");
    if (!stringp(str))
    {
        return 0;
    }

    if (sscanf(str, "%s to %s", intro_who, intro_to) != 2)
    {
        intro_who = str;
    }

    if ((intro_who == "me") || (intro_who == "myself"))
    {
        intro_self = 1;
        introducee = this_player();
    }
    else
    {
        livings = parse_this(intro_who, "[the] %l");
        switch(sizeof(livings))
        {
        case 0:
            return 0;

        case 1:
            break;

        default:
            notify_fail("You can only " + query_verb() +
                " one living at a time.\n");
            return 0;
        }
        introducee = livings[0];

        /* As if Mercade would type 'introduce Mercade' *duh* */
        if (introducee == this_player())
        {
            intro_self = 1;
        }
    }

    if (!intro_self &&
        !(({int}) this_player()->query_met(introducee->query_real_name())))
    {
        notify_fail("You have not been properly introduced to " +
            ({string}) introducee->query_the_name(this_player()) + " yourself.\n");
        return 0;
    }

    if (({int}) introducee->query_prop(OBJ_I_INVIS) > 0)
    {
        notify_fail((intro_self ? "You are " :
            (({string}) introducee->query_The_name(this_player()) + " is ")) +
            "invisible, making introductions impossible.\n");
        return 0;
    }

    livings = FILTER_LIVE(all_inventory(environment(this_player()))) -
        ({ this_player(), introducee });
    if (sizeof(intro_to))
    {
        all_targets = parse_this(intro_to, "[the] %l") - ({ introducee });
        vis_targets = all_targets;
    }
    else
    {
        if (!CAN_SEE_IN_ROOM(this_player()))
        {
            notify_fail("It is way too dark for you to see here.\n");
            return 0;
        }

        all_targets = livings;
        vis_targets = FILTER_CAN_SEE(all_targets, this_player());
    }

    if (!sizeof(vis_targets))
    {
        notify_fail("There is no one to introduce " +
            (intro_self ? "yourself" :
            ({string}) introducee->query_the_name(this_player())) + " to.\n");
        return 0;
    }

    this_player()->reveal_me(1);
    if (!intro_self)
    {
        introducee->reveal_me(1);
    }

    str = ({string}) introducee->query_presentation();
    size = sizeof(all_targets);
    index = -1;
    while(++index < size)
    {
        tell_object(all_targets[index],
            ({string}) this_player()->query_The_name(all_targets[index]) + " introduces " +
            (intro_self ? (({string}) this_player()->query_objective() + "self") :
                ({string}) introducee->query_the_name(all_targets[index])) + " as:\n" +
            str + ".\n");
    }

    if (sizeof(intro_to))
    {
        livings -= all_targets;

        size = sizeof(livings);
        index = -1;
        while(++index < size)
        {
            livings[index]->catch_msg(
                ({string}) this_player()->query_The_name(livings[index]) +
                " introduces " +
                (intro_self ? (({string}) this_player()->query_objective() + "self") :
                    ({string}) introducee->query_the_name(livings[index])) + " to " +
                FO_COMPOSITE_ALL_LIVE(vis_targets, livings[index]) + ".\n");
        }
    }

    if (!intro_self)
    {
        introducee->catch_msg(
            ({string}) this_player()->query_The_name(introducee) +
            " introduces you to " +
            FO_COMPOSITE_ALL_LIVE(vis_targets, introducee) + ".\n");
    }

    if (({int}) this_player()->query_option(OPT_ECHO))
    {
        write("You " + query_verb() + " " + (intro_self ? "yourself" :
            ({string}) introducee->query_the_name(this_player())) + " to " +
            COMPOSITE_ALL_LIVE(vis_targets) + ".\n");
    }
    else
    {
        write("Ok.\n");
    }

    /* Finally, we call the function add_introduced in all the livings
     * that receive the introduction. We do it here so all messages have
     * have been printed, ergo instant-reply can be done.
     */
    all_targets->add_introduced(introducee->query_real_name());

    return 1;
}


/*
 * introduced - Give a list of livings we have been introduced to.
 */
int introduced_list(string str)
{
    object ob;
    mapping tmp;

    tmp = ({mapping}) this_player()->query_introduced();
    if (mappingp(tmp))
    {
        write("You remember having been introduced to:\n");
        write(implode(map(sort_array(m_indices(tmp), #'>), #'capitalize), ", ") + "\n");

        return 1;
    }
    else
    {
        write("You don't remember having been introduced to anyone.\n");
        return 1;
    }
}


/*
 * invite - Invite someone to join my team
 */
int invite(string str)
{
    return team("invite" + (sizeof(str) ? (" " + str) : ""));
}


/*
 * join - Join someones team
 */
varargs int join(string str)
{
    return team("join" + (sizeof(str) ? (" " + str) : ""));
}


/*
 * kill - Start attacking someone with the purpose to kill
 */
varargs int kill(string str)
{
    object ob;
    mixed  property_value, *oblist; // Renamed tmp to property_value
    object me = this_player();
    object my_env = environment(me);
    string ob_s_the_name;
    string me_s_the_name_ob;
    string ob_s_real_name;

    if (!CAN_SEE_IN_ROOM(me))
    {
        notify_fail("You can't see anything here.\n");
        return 0;
    }

    if (({int}) me->query_ghost())
    {
        notify_fail("Umm yes, killed. That's what you are.\n");
        return 0;
    }

    if (!sizeof(str))
    {
        notify_fail("Kill what?\n");
        return 0;
    }

    str = lower_case(str);

    if (!parse_command(str, all_inventory(my_env),
       "[the] %i", oblist) || !sizeof(oblist = NORMAL_ACCESS(oblist, 0, 0)))
    {
        notify_fail("You find no such living creature.\n");
        return 0;
    }

    if (sizeof(oblist) > 1)
    {
        notify_fail("Be specific, you cannot kill " +
            COMPOSITE_ALL_LIVE(oblist) + " at the same time.\n");
        return 0;
    }

    ob = oblist[0];

    if (!living(ob))
    {
       write(capitalize(LANG_THESHORT(ob)) + " isn't alive!\n");
       return 1;
    }

    ob_s_the_name = ({string}) ob->query_the_name(me);
    me_s_the_name_ob = ({string}) me->query_The_name(ob);
    ob_s_real_name = ({string}) ob->query_real_name();

    if (({int}) ob->query_ghost())
    {
        write(smart_capitalize_name(ob_s_the_name) + " is already dead!\n");
        return 1;
    }

    if (ob == me)
    {
        write("What? Attack yourself?\n");
        return 1;
    }

    if (({object}) me->query_attack() == ob)
    {
        write("Yes, yes.\n");
        return 1;
    }

    if (property_value = ({mixed}) my_env->query_prop(ROOM_M_NO_ATTACK))
    {
        if (stringp(property_value))
            write(property_value);
        else
            write("You sense a divine force preventing your attack.\n");
        return 1;
    }

    if (property_value = ({mixed}) ob->query_prop(OBJ_M_NO_ATTACK))
    {
        if (stringp(property_value))
        {
            write(property_value);
        }
        else
        {
            write("You feel a divine force protecting " +
                ob_s_the_name + ", your attack fails.\n");
        }

        return 1;
    }

    if (ob in ({object *}) me->query_team_others())
    {
        write("You cannot attack " + ob_s_the_name +
            " as " + ({string}) ob->query_pronoun() + " is in your team.\n");
        return 1;
    }

    if (!({int}) me->query_npc() &&
        ({int}) me->query_met(ob) &&
        (({object}) me->query_prop(LIVE_O_LAST_KILL) != ob))
    {
        me->add_prop(LIVE_O_LAST_KILL, ob);

        /* Only ask if the person did not use the real name of the target. */
        if (str != ob_s_real_name)
        {
            write("Attack " + ob_s_the_name + "?!? Please confirm by trying again.\n");
            return 1;
        }
    }

    me->reveal_me(1);

    /* Check if we dare! */
    if (!F_DARE_ATTACK(me, ob))
    {
        write("Umm... no. You do not have enough self-discipline to dare!\n");
        say(QCTNAME(me) + " considers attacking " + QTNAME(ob) +
            ", though does not dare to do so.\n", ({ ob, me }) );
        tell_object(ob, me_s_the_name_ob + " eyes you critically.\n");
        return 1;
    }

    say(QCTNAME(me) + " attacks " + QTNAME(ob) + ".\n", ({ me, ob }) );
    tell_object(ob, me_s_the_name_ob + " attacks you!\n");

    me->attack_object(ob);
    me->add_prop(LIVE_O_LAST_KILL, ob);

    if (({int}) me->query_option(OPT_ECHO))
    {
        write("You attack " + ob_s_the_name + ".\n");
    }
    else
    {
        write("Ok.\n");
    }

    return 1;
}


/*
 * last - display information on when a player was last logged in.
 */
int last(string str)
{
    object player;
    int duration;
    int npc;

    if (!stringp(str))
    {
        str = ({string}) this_player()->query_real_name();
    }
    else
    {
        str = lower_case(str);
        if (!(({int}) this_player()->query_met(str)))
        {
            notify_fail("You do not know anyone called " + capitalize(str) +
                ".\n");
            return 0;
        }
    }

    if (({int}) SECURITY->query_wiz_rank(str))
    {
        notify_fail("The command 'last' does not function on wizards.\n");
        return 0;
    }

/*
 * Another feature that is disabled for now because players abuse it to make
 * a rush for equipment of NPC's that introduce themselves.
 *
    if (objectp(player = find_living(str)) &&
        player->query_npc())
    {
        write("Login time : " + ctime(SECURITY->query_start_time() +
            (random(time() - SECURITY->query_start_time()) / 2)) + "\n");
        write("Activity   : not idle\n");
        return 1;
    }
 */

    if (objectp(player = find_player(str)))
    {
        write("Login time : " + ctime(({int}) player->query_login_time()) + "\n");

        if (player == this_player())
        {
            write("Duration   : " +
                TIME2STR((time() - ({int}) player->query_login_time()), 2) + "\n");
        }
        else if (interactive(player))
        {
            if (interactive_info(player, II_IDLE) > 60)
            {
                write("Activity   : " + TIME2STR(interactive_info(player, II_IDLE), 2) +
                    " idle\n");
            }
            else
            {
                write("Activity   : not idle\n");
            }
        }
        else
        {
            write("Activity   : linkdead for " +
                TIME2STR((time() - ({int}) player->query_linkdead()), 2) + "\n");
        }

        return 1;
    }

    if (!(({int}) SECURITY->exist_player(str)))
    {
        write("A player by that name cannot be found in the realms.\n");
        return 1;
    }

    player = ({object}) SECURITY->finger_player(str);
    write("Login time : " + ctime(({int}) player->query_login_time()) + "\n");
    duration = (({int}) player->query_logout_time() - ({int}) player->query_login_time());
    if (duration < 86400)
    {
        write("Logout time: " + ctime(({int}) player->query_logout_time()) + "\n");
        write("Duration   : " + TIME2STR(duration, 3) + "\n");
    }
    else
    {
        write("Logout time: unknown\n");
    }

    player->remove_object();
    return 1;
}


/*
 * leave - Leave a team or force someone to leave a team
 */
int leave(string str)
{
    if (str == "team")
    {
        return team("leave");
    }

    notify_fail("Leave what? Your team?\n");
    return 0;
}


/*
 * remember - Remember one of the livings introduced to us
 */
int remember_live(string str)
{
    object ob;
    mapping tmp;
    int num;

    if (!stringp(str) ||
        query_verb() == "remembered")
    {
        tmp = ({mapping}) this_player()->query_remembered();
        if (mappingp(tmp))
        {
            if (num = sizeof(tmp))
            {
                num = F_MAX_REMEMBERED(({int}) this_player()->query_stat(SS_INT),
                    ({int}) this_player()->query_stat(SS_WIS)) - num;
                if (num < 0)
                    num = 0;

                write("These are the people you remember:\n");
                write(implode(map(sort_array(m_indices(tmp), #'>),
                    #'capitalize), ", ") + "\n");
                write("Your brain can handle " + LANG_WNUM(num) +
                        " more name" + (num == 1 ? ".\n" : "s.\n") );
                return 1;
            }
            else
            {
                write("You do not seem to remember anyone.\n");
                return 1;
            }
        }
        else
        {
            write("You don't remember knowing anyone at all.\n");
            return 1;
        }
    }

    str = lower_case(str);

    /* Silly people remembering themselves can get problems with 'who'. */
    if (({string}) this_player()->query_real_name() == str)
    {
        notify_fail("Sure, as if you would forget yourself if you did " +
            "not remember your name.\n");
        return 0;
    }

    if (objectp(ob = find_living(str)) &&
        (({int}) ob->query_prop(LIVE_I_NON_REMEMBER)))
    {
        notify_fail("Remember " + ({string}) ob->query_objective() + "? Never!\n");
        return 0;
    }

    switch (({int}) this_player()->add_remembered(str))
    {
    case -1:
        write("Your poor brain cannot handle any more people.\n");
        return 1;
    case 1:
        write("Ok.\n");
        return 1;
    case 2:
        write("You refresh your memory of " + capitalize(str) + ".\n");
        return 1;
    default:
        notify_fail("You can't remember having been introduced to " +
                    capitalize(str) + ".\n");
        return 0;
    }
}


/*
 * spar - fight someone for practice.
 */
int spar(string str)
{
    object *oblist;
    int index;
    int size;

    oblist = parse_this(str, "[with] [the] %l");
    if (!(size = sizeof(oblist)))
    {
        notify_fail("Spar with whom?\n");
        return 0;
    }

    index = -1;
    while(++index < size)
    {
        if (({int}) this_player()->query_sparring_partner(oblist[index]))
        {
            write("You are already sparring with " +
                ({string}) oblist[index]->query_the_name(this_player()) + ".\n");
            continue;
        }

        if (({int}) oblist[index]->query_sparring_partner(this_player()))
        {
            write("You accept the challenge to spar with " +
                ({string}) oblist[index]->query_the_name(this_player()) + ".\n");
            tell_object(oblist[index],
                ({string}) this_player()->query_The_name(oblist[index]) +
                " accepts your challenge to spar with " +
                ({string}) this_player()->query_objective() + ".\n");
            tell_room(QCTNAME(this_player()) +
                " accepts the challenge to spar with " +
                QTNAME(oblist[index]) + ".\n", ({ this_player(), oblist }) );
        }
        else
        {
            write("You challenge " +
                ({string}) oblist[index]->query_the_name(this_player()) +
                " to spar with you.\n");
            tell_object(oblist[index],
                ({string}) this_player()->query_The_name(oblist[index]) +
                " challenges you to spar with " +
                ({string}) this_player()->query_objective() + ".\n");
            tell_room(QCTNAME(this_player()) +
                " challenges to spar with " +
                QTNAME(oblist[index]) + ".\n", ({ this_player(), oblist }) );
        }
        this_player()->add_sparring_partner(oblist[index]);
    }
    return 1;
}


/*
 * stop - Stop fighting
 */

/*
 * Function name: remove_stop_fighting_offer
 * Description  : Called to make sure the offer is revoked after 10 seconds.
 * Arguments    : object live - who made the offer.
 *                string str  - the target to which we made the offer.
 */
static void remove_stop_fighting_offer(object live, string str)
{
    mapping offers = ({mixed}) live->query_prop(LIVE_M_STOP_FIGHTING);

    /* There is no mapping. */
    if (!mappingp(offers))
    {
        return;
    }

    /* The offer is still valid, do not touch. */
    if (offers[str] >= (time() - 10))
    {
        return;
    }

    /* Revoke the offer. */
    offers = m_delete(offers, str);
    if (sizeof(offers))
    {
        live->add_prop(LIVE_M_STOP_FIGHTING, offers);
    }
    else
    {
        live->remove_prop(LIVE_M_STOP_FIGHTING);
    }
}


int stop(string str)
{
    object *oblist;
    mapping offers;

    if (!sizeof(str))
    {
        notify_fail("Stop what? Fighting? Counting? The world?\n");
        return 0;
    }

    /* Player wants to stop fighting his current enemy. */
    if (str == "fighting")
    {
        oblist = ({ ({object}) this_player()->query_attack() });
        if (!objectp(oblist[0]))
        {
            notify_fail("You are already as peaceful as can be.\n");
            return 0;
        }
    }
    else
    {
        oblist = parse_this(str, "'fighting' [the] %l");
        switch(sizeof(oblist))
        {
        case 0:
            notify_fail("Stop fighting who?\n");
            return 0;

        case 1:
            break;

        default:
            notify_fail("Stop fighting one person at a time, please?\n");
            return 0;
        }
    }

    /* See if someone offered to stop fighting us. This offer must be accepted
     * within 10 seconds after the offer was made.
     */
    offers = ({mixed}) oblist[0]->query_prop(LIVE_M_STOP_FIGHTING);
    str = object_name(this_player());
    if (mappingp(offers) &&
        (offers[str] >= (time() - 10)))
    {
        offers = m_delete(offers, str);
        if (sizeof(offers))
        {
            oblist[0]->add_prop(LIVE_M_STOP_FIGHTING, offers);
        }
        else
        {
            oblist[0]->remove_prop(LIVE_M_STOP_FIGHTING);
        }

        write("You accept the offer to cease hostilities with " +
            ({string}) oblist[0]->query_the_name(this_player()) + " and stop fighting " +
            ({string}) oblist[0]->query_objective() + ".\n");
        tell_object(oblist[0], ({string}) this_player()->query_The_name(oblist[0]) +
            " accepts your offer to cease hostilities and stops fighting " +
            "you.\n");
        say(QCTNAME(this_player()) + " accepts the offer of " +
            QTNAME(oblist[0]) + " to cease hostilities and they stop " +
            "fighting each other.\n", ({ this_player(), oblist[0] }));

        this_player()->stop_fight(oblist[0]);
        oblist[0]->stop_fight(this_player());
        return 1;
    }

    /* Before we offer, are we fighting him? */
    if (oblist[0] in ({object *}) this_player()->query_enemy(-1))
    {
        write("You are not fighting " +
            ({string}) oblist[0]->query_the_name(this_player()) + ".\n");
        return 1;
    }

    offers = ({mapping}) this_player()->query_prop(LIVE_M_STOP_FIGHTING);
    str = object_name(oblist[0]);
    if (!mappingp(offers))
    {
        offers = ([ ]);
    }

    /* Offer to cease hostilities, or renew the offer. */
    if (!offers[str])
    {
        write("You offer to cease the hostilities with " +
            ({string}) oblist[0]->query_the_name(this_player()) +
            ", giving " + ({string}) oblist[0]->query_objective() +
            " a chance to stop fighting with you.\n");
        tell_object(oblist[0], ({string}) this_player()->query_The_name(oblist[0]) +
            " offers to cease the hostilities with you, giving you the " +
            "chance to stop fighting with " +
            ({string}) this_player()->query_objective() + " if you so choose. To " +
            "accept this offer, you must indicate that you also wish to " +
            "stop fighting within a short period of time.\n");
        say(QCTNAME(this_player()) + " offer to cease hostilities with " +
            QTNAME(oblist[0]) + " and thus to stop fighting each other.\n",
            ({ this_player(), oblist[0] }));
    }
    else
    {
        write("You renew your offer to cease the hostilities with " +
            ({string}) oblist[0]->query_the_name(this_player()) +
            ", giving " + ({string}) oblist[0]->query_objective() +
            " a chance to stop fighting with you.\n");
        tell_object(oblist[0], ({string}) this_player()->query_The_name(oblist[0]) +
            " renews " + ({string}) this_player()->query_possessive() + " offer to " +
            "cease the hostilities with you, giving you the chance to " +
            "stop fighting with " + ({string}) this_player()->query_objective() +
            " if you so choose. To accept this offer, you must indicate " +
            "that you also wish to stop fighting within a short period " +
            "of time.\n");
        say(QCTNAME(this_player()) + " renews " +
            ({string}) this_player()->query_possessive() + " offer to cease " +
            "hostilities with " + QTNAME(oblist[0]) + " and thus to stop " +
            "fighting each other.\n", ({ this_player(), oblist[0] }));
    }

    /* Register the offer. */
    offers[str] = time();
    this_player()->add_prop(LIVE_M_STOP_FIGHTING, offers);

    /* Purposely remove after 15 and not 10 seconds. */
    call_out("remove_stop_fighting_offer", 15,
        ({ this_player(), str }) );

    /* Call the hook after all messages are printed. */
    oblist->hook_stop_fighting_offer(this_player());
    return 1;
}


/*
 * team - Handles all the team related commands.
 */

#define FAIL_IF_LEADER(text) if (leader) { notify_fail("You cannot " + (text) + " as you are lead by " + ({string}) leader->query_the_name(this_player()) + ".\n"); return 0; }
#define FAIL_IF_NOT_LEADER(text) if (!sizeof(members)) { notify_fail("You cannot " + (text) + " as you are not leading a team.\n"); return 0; }


static int team_invite(object *oblist)
{
    object *npcs = ({ });
    string fail = "";

    this_player()->reveal_me(1);
    npcs = filter(oblist, (: ({int}) $1->query_npc() :));
    if (sizeof(npcs))
    {
        fail = capitalize(COMPOSITE_ALL_LIVE(npcs)) + " decline" +
            ((sizeof(npcs) == 1) ? "s" : "") +
            " the invitation to join your team.\n";
        oblist -= npcs;
    }

    npcs = filter(oblist, (: ({object}) $1->query_leader() == this_player() :));
    if (sizeof(npcs))
    {
        fail += capitalize(COMPOSITE_ALL_LIVE(npcs)) +
            ((sizeof(npcs) == 1) ? " is" : " are") + " already in your team.\n";
        oblist -= npcs;
    }

    if (!sizeof(oblist))
    {
        notify_fail(fail);
        return 0;
    }

    map(oblist, (: ({int}) $1->reveal_me(1) :));
    foreach(object ob: oblist)
    {
        this_player()->team_invite(ob);
    }

    write(fail + "You invite " + COMPOSITE_ALL_LIVE(oblist) +
        " to join your team.\n");
    targetbb(" invites you to join " + ({string}) this_player()->query_possessive() +
        " team.", oblist);
    all2actbb(" invites", oblist, " to join " +
        ({string}) this_player()->query_possessive() + " team.");
    return 1;
}


static int team_join(object leader)
{
    if (sizeof(({object *}) this_player()->query_team()))
    {
        write("You cannot join a team while you are leading a team.\n");
        return 1;
    }
    if (!(this_player() in ({object *}) leader->query_invited()))
    {
        write(({string}) leader->query_The_name(this_player()) +
            " has not invited you as a team member.\n");
        return 1;
    }
    if (({int}) leader->query_leader())
    {
        write("You cannot join " + ({string}) leader->query_the_name(this_player()) +
            " as " + ({string}) leader->query_pronoun() + " joined a team " +
            ({string}) leader->query_objective() + "self.\n");
        return 1;
    }
    if (!({int}) leader->team_join(this_player()))
    {
        write("You fail to join your leader.\n");
        return 1;
    }

    if (!({int}) this_player()->query_option(OPT_BRIEF))
    {
        write("As you enter the team, you switch to brief mode.\n");
        this_player()->add_prop(TEMP_BACKUP_BRIEF_OPTION, 1);
        this_player()->set_option(OPT_BRIEF, 1);
    }

    write("Your leader is now " + ({string}) leader->short() + ".\n");
    say(QCTNAME(this_player()) + " joins the team of " +
        QTNAME(leader) + ".\n", ({ leader, this_player() }));
    tell_object(leader, ({string}) this_player()->query_The_name(leader) +
        " joins your team.\n");
    return 1;
}


static void team_leave(object member, object leader, int force)
{
    leader->team_leave(member);

    /* In case of force, don't allow the person to join again. */
    if (force)
    {
        leader->remove_invited(member);
    }

    if (({int}) member->query_prop(TEMP_BACKUP_BRIEF_OPTION))
    {
        tell_object(member, "As you leave the team, you switch back to " +
            "verbose mode.\n");
        member->remove_prop(TEMP_BACKUP_BRIEF_OPTION);
        member->set_option(OPT_BRIEF, 0);
    }
}


varargs int team(string str)
{
    string  arg = "";
    int     done;
    int     size = 0;
    object *oblist = ({ });
    object  leader = ({object}) this_player()->query_leader();
    object  rear;
    object  member;
    object *members = ({object *}) this_player()->query_team();

    if (!sizeof(str))
    {
        str = "list";
    }

    if (sscanf(str, "%s %s", str, arg) == 2)
    {
        oblist = parse_this(arg, "[the] %l");
        size = sizeof(oblist);
    }

    switch(str)
    {
    case "disband":
        FAIL_IF_NOT_LEADER("disband your team");
        write("You disband your team.\n");
	members->catch_msg(QCTNAME(this_player()) + " disbands " +
        ({string}) this_player()->query_possessive() + " team and forces you to leave.\n");
        members = FILTER_PRESENT(members);
        all2actbb(" disbands " + ({string}) this_player()->query_possessive() +
            " team, forcing", members, " to leave.");

        map(members, (: team_leave($1, this_player(), 1) :));
        return 1;

    case "invite":
        FAIL_IF_LEADER("invite anyone");
        if (!size)
        {
            notify_fail("Invite who to your team?\n");
            return 0;
        }
        return team_invite(oblist);

    case "invited":
        oblist = ({object *}) this_player()->query_invited();
        if (sizeof(oblist))
        {
            write("You have invited " + COMPOSITE_ALL_LIVE(oblist) +
                " to join your team.\n");
            done = 1;
        }
        oblist = filter(users(), (: member(({object *}) $1->query_invited(), this_player()) > -1 :));
        if (sizeof(oblist))
        {
            write("You have been invited to join the team of " +
                COMPOSITE_ALL_LIVE(oblist) + ".\n");
            done = 1;
        }
        if (!done)
        {
            write("You have neither invited anyone to join your team, " +
                "nor been invited to join any team.\n");
        }
        return 1;

    case "join":
        FAIL_IF_LEADER("join another team");
        switch(size)
        {
        case 0:
            notify_fail("Whose team do you want to join?\n");
            return 0;
        case 1:
            break;
        default:
            notify_fail("You can join only one team at a time.\n");
            return 0;
        }
        return team_join(oblist[0]);

    case "leader":
        FAIL_IF_NOT_LEADER("assign a new team leader");
        if (({int}) this_player()->query_attack())
        {
            notify_fail("You cannot re-arrange your team while you are in combat.\n");
            return 0;
        }
        switch(size)
        {
        case 0:
            notify_fail("Assign who to be the leader of your team?\n");
            return 0;
        case 1:
            leader = oblist[0];
            break;
        default:
            notify_fail("You can assign only a single leader.\n");
            return 0;
        }

        if (!(leader in members))
        {
            write(({string}) rear->query_The_name(this_player()) +
                " is not a member of your team.\n");
            return 1;
        }

        /* Remove the members from the old leader (me).
	 * Note: both this and the next foreach are written as foreach instead
	 * of map as a functionpointer (as used in map)  cannot be shadowed. */
        foreach(object ob: members)
        {
            this_player()->team_leave(ob);
        }
        members += ({ this_player() });
        members -= ({ leader });
        /* Add the members to the new leader. */
        foreach(object ob: members)
        {
            leader->team_join(ob);
        }

        done = ({int}) this_player()->query_option(OPT_BRIEF);
        write("You make " + ({string}) leader->query_the_name(this_player()) +
            " the leader of your team" +
            (done ? "" : " and switch into brief mode") + ".\n");
        if (!done)
        {
            this_player()->add_prop(TEMP_BACKUP_BRIEF_OPTION, 1);
            this_player()->set_option(OPT_BRIEF, 1);
        }
        leader->catch_tell(({string}) this_player()->query_The_name(leader) +
            " makes you the leader of " + ({string}) this_player()->query_possessive() + " team.\n");
        if (({int}) leader->query_prop(TEMP_BACKUP_BRIEF_OPTION))
        {
            tell_object(leader, "As you lead the team, you switch back to verbose mode.\n");
            leader->remove_prop(TEMP_BACKUP_BRIEF_OPTION);
            leader->set_option(OPT_BRIEF, 0);
        }
        all2actbb(" makes", ({ leader}), " the leader of " +
            ({string}) this_player()->query_possessive() + " team.");
        members->catch_msg("You are now lead by " + QTNAME(leader) + ".\n");
        return 1;

    case "leave":
        if (objectp(leader))
        {
            if (sizeof(arg))
            {
                notify_fail("Leave your team and what else?\n");
                return 0;
            }
            tell_object(leader, ({string}) this_player()->query_The_name(leader) +
                " leaves your team.\n");
            write("You leave the team of " +
                ({string}) leader->query_the_name(this_player()) + ".\n");
            say(QCTNAME(this_player()) + " leaves the team of " +
                QTNAME(leader) + ".\n", ({ leader, this_player() }));
            team_leave(this_player(), leader, 0);
            return 1;
        }
        FAIL_IF_NOT_LEADER("remove someone from your team");
        if (!sizeof(oblist))
        {
            if (!sizeof(arg))
            {
                notify_fail("You are leading the team. " +
                    "To disband it, please \"team disband\".\n");
                return 0;
            }
            member = find_player(arg);
            if (!objectp(member) || !({string}) (this_player()->query_met(member)))
            {
                notify_fail("Remove whom from your team?\n");
                return 0;
            }
            oblist = ({ member });
        }

        map(oblist, (: team_leave($1, this_player(), 1) :));
        write("You force " + COMPOSITE_ALL_LIVE(oblist) + " to leave your team.\n");
        oblist->catch_msg(QCTNAME(this_player()) + " forces you to leave " +
            ({string}) this_player()->query_possessive() + " team.\n");
        oblist = FILTER_PRESENT(oblist);
        all2actbb(" forces", oblist, " to leave " +
            ({string}) this_player()->query_possessive() + " team.");
        return 1;

    case "list":
        if (leader)
        {
            members = ({object *})leader->query_team() - ({ this_player() });
            write("The leader of your team is " +
                ({string}) leader->query_the_name(this_player()) + ". ");
            switch(sizeof(members))
            {
            case 0:
                write("You are the only member.\n");
                return 1;
            case 1:
                write("The other member is " + ({string}) members[0]->query_the_name(this_player()) + ".\n");
                return 1;
            default:
                write("The other members are " + COMPOSITE_ALL_LIVE(members) + ".\n");
                return 1;
            }
            /* Not reached. */
        }
        if (sizeof(members))
        {
            write("You are the leader of your team. The members are " +
                COMPOSITE_ALL_LIVE(members) + ".\n");
            return 1;
        }
        notify_fail("You are not in a team.\n");
        return 0;

    case "rear":
        FAIL_IF_NOT_LEADER("assign a new rear guard");
        switch(size)
        {
        case 0:
            notify_fail("Who do you want to be the rear guard?\n");
            return 0;
        case 1:
            rear = oblist[0];
            break;
        default:
            notify_fail("You can assign only one person to be the rear guard.\n");
            return 0;
        }

        if (!(rear in members))
        {
            write(({string}) rear->query_The_name(this_player()) +
                " is not a member of your team.\n");
            return 1;
        }
        if (rear == members[sizeof(members)-1])
        {
            write(({string}) rear->query_The_name(this_player()) +
                " already is the rearguard of the team.\n");
            return 1;
        }

        this_player()->team_leave(rear);
        this_player()->team_join(rear);
        members -= ({ rear });

        write("You alter the formation of the team, placing "+
            ({string}) rear->query_the_name(this_player()) + " at the rearguard.\n");
        all2actbb(" alters the formation of " + ({string}) this_player()->query_possessive() +
            " team, placing", ({ rear }), " at the rearguard.");
        rear->catch_tell(({string}) this_player()->query_The_name(rear) +
            " places you at the rearguard of " + ({string}) this_player()->query_possessive() +
            " team.\n");
        return 1;

    default:
        notify_fail("The command \"team " + str + "\" is not know.\n");
        return 0;
    }

    write("Impossible end of team command.\n");
    return 1;
}


/*
 * who - Tell what players are logged in and who we know
 */

/*
 * Function name: index_arg
 * Description  : This function returns whether a particular letter is
 *                used in the argument the player passed to the function.
 * Arguments    : string str    - the arguments.
 *                string letter - the letter to search for.
 * Returns      : int 1/0 - true if the letter is used.
 */
nomask int index_arg(string str, string letter)
{
    return (member(letter, explode(str, "")) != -1);
}


/*
 * Function name: get_name
 * Description  : This map function will return the name of the player for
 *                the 'who n' command. If the living is linkdead, an
 *                asterisk (*) is added.
 * Arguments    : object player - the player to return the name for.
 * Returns      : string - the name to print.
 */
nomask string get_name(object player)
{
    string name = capitalize(({string}) player->query_real_name());

    /* If the player is linkdead, we add an asterisk (*) to the name. */
    if (!interactive(player) &&
        !({int}) player->query_npc())
    {
        name += "*";
    }

    return name;
}


/*
 * Function name: print_who
 * Description  : This function actually prints the list of people known.
 * Arguments    : string opts  - the command line arguments.
 *                object *list - the list of livings to display.
 *                int    size  - the number of people logged in.
 * Returns      : int 1 - always.
 */
nomask int print_who(string opts, object *list, int size)
{
    int i, j;
    int scrw = ({int}) this_player()->query_option(OPT_SCREEN_WIDTH);
    string to_write = "";
    string *title;
    string tmp;
    int mwho = (query_verb() == "mwho");

    scrw = (scrw ? scrw : 80);
    list = sort_array(list, #'sort_name);

    if (!sizeof(list))
    {
        to_write += ("There are no players of the requested type present " +
            "that you know.\n");
        /* No need to check for mwho here. */
        write(to_write);
        return 1;
    }

    if (size == 1)
    {
        to_write += ("Only one player present.\n");
    }
    else
    {
        to_write += ("There are " + size +
            " players in the game. Within the requested type you know:\n");
    }

    /* By default we display only the names, unless the argument 'f' for
     * full was given.
     */
    if (!index_arg(opts, "f"))
    {
        scrw = ((scrw >= 40) ? (scrw - 3) : 77);
        to_write += (sprintf("%-*#s\n", scrw,
            implode(map(list, #'get_name), "\n")));
        /* No need to check for mwho here. */
        write(to_write);
        return 1;
    }

    for(i = 0; i < sizeof(list); i++)
    {
        tmp = ({string}) list[i]->query_presentation();
        if (!interactive(list[i]) &&
            !({int}) list[i]->query_npc())
        {
            title = explode(tmp, " ");
            title[(({int}) list[i]->query_wiz_level() ? 1 : 0)] += "*";
            tmp = implode(title, " ");
        }
        if ((scrw == -1) || (sizeof(tmp) < scrw))
        {
            to_write += (tmp + "\n");
        }
        else /* Split a too long title in a nice way. */
        {
            title = explode(tmp, "\n");
            tmp = sprintf("%-*s\n", scrw, title[0]);

            title = explode(
                implode(title[1..], " "), "\n");

            for(j = 0; j < sizeof(title); j++)
                tmp += (sprintf("      %-*s\n", (scrw - 6), title[j]));

            to_write += (tmp);
        }
    }

    /* Too long message is not what we want. */
    if (!mwho &&
        (sizeof(to_write) > 4000))
    {
        write("The list is too long. More automatically invoked.\n");
        mwho = 1;
    }

    if (mwho)
    {
        this_player()->more(to_write);
    }
    else
    {
        write(to_write);
    }

    return 1;
}


/*
 * Function name: sort_name
 * Description  : This sort function sorts on the name of the player. Since
 *                no two players can have the same name, we do not have to
 *                check for that.
 * Arguments    : object a - the playerobject to player a.
 *                object b - the playerobject to player b.
 * Returns      : int -1 - name of player a comes before that of player b.
 *                     1 - name of player b comes before that of player a.
 */
nomask int sort_name(object a, object b)
{
    string aname = ({string}) a->query_real_name();
    string bname = ({string}) b->query_real_name();

    return ((aname == bname) ? 0 : ((aname < bname) ? -1 : 1));
}


/*
 * Function name: filter_who_no_invis_wizard
 * Description  : Filters out all invisible wizards.
 * Arguments    : object player - the player to test.
 * Returns      : int 1/0 - TRUE when not an invis wizard.
 */
int filter_who_no_invis_wizard(object player)
{
    return (!({int}) player->query_wiz_level() ||
        (({int}) player->query_prop(OBJ_I_INVIS) < 100));
}


int who(string opts)
{
    object  *list = users();
    object  npc;
    mapping rem;
    mapping memory = ([ ]);
    string  *names = ({ });
    int     index;
    int     size;
    int     size_list;

    if (!stringp(opts))
    {
        opts = "";
    }

#ifdef STATUE_WHEN_LINKDEAD
#ifdef OWN_STATUE
    /* If there is a room where statues of linkdead people can be found,
     * we add that to the list, but only if the player did not ask to only
     * see the interactive players.
     */
    if (!index_arg(opts, "i") &&
        objectp(find_object(OWN_STATUE)))
    {
        list += (({object *})find_object(OWN_STATUE)->query_linkdead_players() - list);
    }
#endif
#endif

    /* This filters out players logging in and such. */
    list = filter(list, (: function_exists("create_container", $1) == LIVING_OBJECT :));
    size = sizeof(list);

    /* Player may indicate to see only wizards or mortals. */
    if (index_arg(opts, "w"))
    {
        list = filter(list, (: ({int}) $1->query_wiz_level() :));
    }
    else if (index_arg(opts, "m"))
    {
        list = filter(list, (: !({int}) $1->query_wiz_level() :));
    }

    /* Wizards won't see the NPC's and wizards are not subject to the
     * met/nonmet system if that is active.
     */
    if (({int}) this_player()->query_wiz_level())
    {
        return print_who(opts, list, size);
    }

    if (mappingp(rem = ({mapping}) this_player()->query_remembered()))
    {
        memory += rem;
    }
    if (mappingp(rem = ({mapping}) this_player()->query_introduced()))
    {
        memory += rem;
    }

    /* Player wants to see who is in the queue. */
    if (index_arg(opts, "q"))
    {
        names = ({string *}) QUEUE->queue_list(1);
        if (!(size = sizeof(names)))
        {
            write("There are no players in the queue right now.\n");
            return 1;
        }

        index = -1;
        while(++index < size)
        {
            names[index] = sprintf("%2d: %-11s", (index + 1),
                (memory[names[index]] ? capitalize(names[index]) :
                 "<unknown>"));
        }
        write("The following people are in the queue:\n" +
              sprintf("%-70#s\n", implode(names, "\n")));
        return 1;
    }

#ifdef MET_ACTIVE
    index = -1;
    size_list = sizeof(list);
    while(++index < size_list)
    {
        if ((!(({string}) memory[list[index]->query_real_name()])) &&
            (!(({int}) list[index]->query_prop(LIVE_I_ALWAYSKNOWN))))
        {
            list[index] = 0;
        }
    }

    list = list - ({ 0, this_player() });
#endif

    /* Don't add NPC's if the player wanted wizards. Here we also add the
     * player himself again, because that is lost during the met-check (when
     * enabled).
     */
    if (!index_arg(opts, "w"))
    {
#ifdef MET_ACTIVE
        list += ({ this_player() });
        size++;
#endif
        names = m_indices(memory) - ({mixed}) list->query_real_name();

#ifdef NPC_IN_WHO_LIST
        index = -1;
        size_list = sizeof(names);
        while(++index < size_list)
        {
            /* We check that the people found this way are NPC's since
             * we do not want linkdead people to show up this way They
             * are already in the list.
             */
            if (objectp(npc = find_living(names[index])) &&
            ({int}) npc->query_npc())
            {
                list += ({ npc });
                size++;
            }
        }
#endif
    }

    /* To mortals 'who' will not show invisible wizards. */
    list = filter(list, #'filter_who_no_invis_wizard);

    return print_who(opts, list, size);
}
