/*
 * /cmd/live/state.c
 *
 * General commands for finding out a livings state.
 * The following commands are:
 *
 * - adverbs
 * - compare
 * - email
 * - h
 * - health
 * - levels
 * - options
 * - second
 * - skills
 * - stats
 * - v
 * - vitals
 */

#pragma no_clone
#pragma no_inherit
#pragma strict_types

inherit "/std/command_driver";

#include <adverbs.h>
#include <cmdparse.h>
#include <composite.h>
#include <const.h>
#include <libfiles.h>
#include <filter_funs.h>
#include <formulas.h>
#include <language.h>
#include <login.h>
#include <mail.h>
#include <macros.h>
#include <options.h>
#include <ss_types.h>
#include <state_desc.h>
#include <std.h>
#include <stdproperties.h>
#include <libtime.h>
#include <configuration.h>

#define SUBLOC_MISCEXTRADESC "_subloc_misc_extra"

/*
 * Global constants
 */
private mixed beauty_strings, skillmap, compare_strings;
private string *stat_names, *health_state, *mana_state, *enc_weight;
private string *intox_state, *stuff_state, *soak_state, *improve_fact;
private string *brute_fact, *panic_state, *fatigue_state;
private mapping lev_map;

/* Prototype */
public int second(string str);


void create()
{
    configure_object(this_object(), OC_EUID, getuid());

    /* These global arrays are created once for all since they are used
       quite often. They should be considered constant, so do not mess
       with them
    */

    skillmap =        SS_SKILL_DESC;

    stat_names =      SD_STAT_NAMES;

    compare_strings = ({ SD_COMPARE_STR, SD_COMPARE_DEX, SD_COMPARE_CON,
                         SD_COMPARE_INT, SD_COMPARE_WIS, SD_COMPARE_DIS,
                         SD_COMPARE_HIT, SD_COMPARE_PEN, SD_COMPARE_AC });

    beauty_strings =  ({ SD_BEAUTY_FEMALE, SD_BEAUTY_MALE });

    brute_fact =      SD_BRUTE_FACT;
    health_state =    SD_HEALTH;
    mana_state =      SD_MANA;
    panic_state =     SD_PANIC;
    fatigue_state =   SD_FATIGUE;
    soak_state =      SD_SOAK;
    stuff_state =     SD_STUFF;
    intox_state =     SD_INTOX;
    improve_fact =    SD_IMPROVE;
    enc_weight =      SD_ENC_WEIGHT;

    lev_map =         SD_LEVEL_MAP;
}


/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
string get_soul_id()
{
    return "state";
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
              "adverbs":"adverbs",

              "compare":"compare",

              "email":"email",

              "h":"health",
              "health":"health",

              "levels":"levels",

              "options":"options",

              "skills":"show_skills",
              "stats":"show_stats",

              "v":"vitals",
              "vitals":"vitals",
            ]);
}


/*
 * Function name: using_soul
 * Description:   Called once by the living object using this soul. Adds
 *                  sublocations responsible for extra descriptions of the
 *                  living object.
 */
public void using_soul(object live)
{
    live->add_subloc(SUBLOC_MISCEXTRADESC, object_name(this_object()));
    live->add_textgiver(object_name(this_object()));
}


/* **************************************************************************
 * Here follows some support functions.
 * **************************************************************************/

/*
 * Function name: beauty_text
 * Description:   Return corresponding text to a certain combination of
 *                appearance and opinion
 */
public string beauty_text(int num, int sex)
{
    if (sex != 1)
        sex = 0;

    return GET_PROC_DESC(num, beauty_strings[sex]);
}


public string show_subloc_size(object on, object for_obj)
{
    string race, res;
    int val, rval, *proc;

    race = ({string}) on->query_race();

    if (race in RACES)
    {
        val = ({int}) on->query_prop(CONT_I_HEIGHT);
        rval = RACEATTR[race][0];
        val = 100 * val / (rval ? rval : val);
        proc = SPREAD_PROC;

        for (rval = 0; rval < sizeof(proc); rval++)
            if (val <= proc[rval])
                break;
        rval = (rval < sizeof(proc) ? rval : sizeof(proc) -1 );

        res = " " + HEIGHTDESC[rval] + " and ";

        val = ({int}) on->query_prop(CONT_I_WEIGHT) / ({int}) on->query_prop(CONT_I_HEIGHT);
        rval = RACEATTR[race][5];
        val = 100 * val / (rval ? rval : val);
        proc = SPREAD_PROC;

        for (rval = 0; rval < sizeof(proc); rval++)
            if (val <= proc[rval])
                break;
        rval = (rval < sizeof(proc) ? rval : sizeof(proc) -1 );

        res += WIDTHDESC[rval] + " for " + LANG_ADDART(on->query_race_name()) +
            ".\n";
    }
    else
        res = "";

    return res;
}


public string show_subloc_looks(object on, object for_obj)
{
    if (({int}) on->query_prop(NPC_I_NO_LOOKS))
        return "";

    return (for_obj == on ?
                "You look " : capitalize(({string}) on->query_pronoun()) + " looks ") +
                beauty_text(({int}) for_obj->my_opinion(on),
                            (({int}) on->query_gender() == G_FEMALE ? 0 : 1)) + ".\n";
}


public string show_subloc_fights(object on, object for_obj)
{
    object eob;

    eob = ({object}) on->query_attack();

    return " fighting " + (eob == for_obj ? "you" :
                           ({string})eob->query_the_name(for_obj)) + ".\n";
}


public string show_subloc_health(object on, object for_obj)
{
    return GET_NUM_DESC(on->query_hp(), on->query_max_hp(), health_state);
}


/*
 * Function name: show_subloc
 * Description:   Shows the specific sublocation description for a living
 */
public string show_subloc(string subloc, object on, object for_obj)
{
    string res, cap_pronoun, cap_pronoun_verb, tmp;

    if (({int}) on->query_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS))
        return "";

    if (for_obj == on)
    {
        res = "You are";
        cap_pronoun_verb = res;
        cap_pronoun = "You are ";
    }
    else
    {
        res = capitalize(({string}) on->query_pronoun()) + " is";
        cap_pronoun_verb = res;
        cap_pronoun = capitalize(({string}) on->query_pronoun()) + " seems to be ";
    }

    if (sizeof(tmp = show_subloc_size(on, for_obj)))
        res += tmp;
    else
        res = "";

    res += show_subloc_looks(on, for_obj);

    if (({int}) on->query_attack())
        res += cap_pronoun_verb + show_subloc_fights(on, for_obj);

    res += cap_pronoun + show_subloc_health(on, for_obj) + ".\n";

    return res;
}


/*
 * Function name: get_proc_text
 * Description  : This is a service function kept for backward compatibility
 *                with existing domain code.
 *                For new code, please use one of the following macros:
 *                 GET_NUM_DESC, GET_NUM_DESC_SUB
 *                 GET_PROC_DESC, GET_PROC_DESC_SUB
 */
public varargs string get_proc_text(int proc, mixed maindescs, int turnindex = 0, mixed subdescs = 0)
{
    if (sizeof(subdescs))
    {
        return GET_PROC_DESC_SUB(proc, maindescs, subdescs, turnindex);
    }
    else
    {
        return GET_PROC_DESC(proc, maindescs);
    }
}


/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the
 * same order as in the function name list.
 * **************************************************************************/

/*
 * Adverbs - list the adverbs available to the game.
 */

/*
 * Function name: write_adverbs
 * Description  : Print the list of found adverbs to the screen of the player
 *                in a nice and tabulated way. It also prints the adverb
 *                replacements suitable for this list.
 * Arguments    : string *adverb_list - the list of adverbs to print.
 *                int total - the total number of adverbs.
 */
void write_adverbs(string *adverb_list, int total)
{
    string  text;
    string *words;
    mapping replacements;
    int     index;
    int     size;

    size = sizeof(adverb_list);
    write("From the total of " + total + " adverbs, " + size +
        (size == 1 ? " matches" : " match") + " your inquiry.\n\n");

    index = -1;
    size = sizeof(ALPHABET);
    while(++index < size)
    {
	words = filter(adverb_list, (: $1 in (ALPHABET[index..index] + "*") :));

	if (!sizeof(words))
	{
	    continue;
	}

	if (sizeof(words[0]) < 16)
	{
	    words[0] = (words[0] + "                ")[..15];
	}
	write(sprintf("%-76#s\n\n", implode(words, "\n")));
    }

    text = "";
    index = -1;
    size = sizeof(adverb_list);
    replacements = ({mapping})ADVERBS_FILE->query_all_adverb_replacements();

    while(++index < size)
    {
        if (sizeof(replacements[adverb_list[index]]))
        {
            text += sprintf("%-16s: %s\n", adverb_list[index],
                replacements[adverb_list[index]]);
        }
    }
    if (sizeof(text))
    {
        write("Of these, the following adverbs are replaced with a more " +
            "suitable phrase:\n\n" + text);
    }
}


int adverbs(string str)
{
    string *adverb_list;
    string *words;
    string *parts;
    mapping replacements;
    int     index;
    int     size;

    if (!sizeof(str))
    {
        notify_fail("Syntax: adverbs all  or  list  or  *\n" +
            "        adverbs <letter>\n" +
            "        adverbs <wildcards>\n" +
            "        adverbs replace[ments]\n" +
            "Commas may be used to specify multiple letters or wildcards.\n");
        return 0;
    }

    /* Player wants to see the replacements only. */
    if (str[..6] == "replace")
    {
        write("\nThe following adverbs are replaced with a more suitable " +
            "phrase:\n\n");
        replacements = ({mapping}) ADVERBS_FILE->query_all_adverb_replacements();
        adverb_list = sort_array(m_indices(replacements), #'<);
        index = -1;
        size = sizeof(adverb_list);
        while(++index < size)
        {
            write(sprintf("%-16s: %s\n", adverb_list[index],
                replacements[adverb_list[index]]));
        }
        return 1;
    }

    adverb_list = ({string *}) ADVERBS_FILE->query_all_adverbs();
    /* Player wants to see all adverbs. */
    if ((str == "list") ||
        (str == "all") ||
        (str == "*"))
    {
        write(".   Use the period to not use any adverb at all, not even " +
            "the default.\n\n");
        write_adverbs(adverb_list, sizeof(adverb_list));
        return 1;
    }

    /* Make lower case, remove spaces and split on commas. */
    parts = explode(implode(explode(lower_case(str), " "), ""), ",");
    index = -1;
    size = sizeof(parts);
    words = ({ });
    while(++index < size)
    {
        if (parts[index] == ".")
        {
            write(".   Use the period to not use any adverb at all, not " +
                "even the default.\n\n");
            continue;
        }
        if (sizeof(parts[index]) == 1)
        {
            parts[index] += "*";
        }
        words |= filter(adverb_list, #'regmatch, "^" + parts[index]);
    }

    if (!sizeof(words))
    {
        write("No adverbs found with those specifications.\n");
        return 1;
    }

    write_adverbs(sort_array(words, #'>), sizeof(adverb_list));
    return 1;
}


/*
 * Compare - compare the stats of two livings, or compare two items.
 */

/*
 * Function name: compare_living
 * Description  : Compares the stats of two livings. The left hand side can
 *                be this_player().
 * Arguments    : object living1 - the left hand side to compare.
 *                object living2 - the right hand side to compare.
 */
void compare_living(object living1, object living2)
{
    int skill = (1000 / (1 + ({int}) this_player()->query_skill(SS_APPR_MON)));
    int seed  = to_int(OB_NUM(living1)) + to_int(OB_NUM(living2));
    int index = -1;
    int stat1;
    int stat2;
    int swap;
    string str1 = ((this_player() == living1) ? "you" :
                    ({string}) living1->query_the_name(this_player()));
    string str2 = ({string}) living2->query_the_name(this_player());

    /* Someone might actually want to compare two livings with the same
     * description.
     */
    if (str1 == str2)
    {
        str1 = "the first " + ({string}) living1->query_nonmet_name();
        str2 = "the second " + ({string}) living2->query_nonmet_name();
    }

    /* Loop over all known stats. */
    while(++index < SS_NO_EXP_STATS)
    {
        stat1 = ({int}) living1->query_stat(index) + random(skill);
        stat2 = ({int}) living2->query_stat(index) + random(skill);

        if (stat1 > stat2)
        {
            stat1 = (100 - ((80 * stat2) / stat1));
            swap = 0;
        }
        else
        {
            stat1 = (100 - ((80 * stat1) / stat2));
            swap = 1;
        }
        stat1 = ((stat1 * sizeof(compare_strings[index])) / 100);
        stat1 = ((stat1 > 3) ? 3 : stat1);

        /* Print the message. */
        if (swap)
        {
            write(capitalize(str2) + " is " +
                compare_strings[index][stat1] + " " + str1 + ".\n");
        }
        else
        {
            write(((str1 == "you") ? "You are " :
                (capitalize(str1) + " is ")) +
                compare_strings[index][stat1] + " " + str2 + ".\n");
        }
    }
}


/*
 * Function name: compare_weapon
 * Description  : Compares the stats of two weapons.
 * Arguments    : object weapon1 - the left hand side to compare.
 *                object weapon2 - the right hand side to compare.
 */
void compare_weapon(object weapon1, object weapon2)
{
    int skill = (2000 / (1 + ({int}) this_player()->query_skill(SS_APPR_OBJ) +
        ({int}) this_player()->query_skill(SS_WEP_FIRST + ({int}) weapon1->query_wt())));
    int seed = to_int(OB_NUM(weapon1)) + to_int(OB_NUM(weapon2));
    int swap;
    int stat1;
    int stat2;
    string str1;
    string str2;
    string print1;
    string print2;
    object tmp;

    /* Always use the same order. After all, we don't want "compare X with Y"
     * to differ from "compare Y with X".
     */
    if (OB_NUM(weapon1) > OB_NUM(weapon2))
    {
        tmp = weapon1;
        weapon1 = weapon2;
        weapon2 = tmp;
        swap = 1;
    }

    str1 = ({string}) weapon1->short(this_player());
    str2 = ({string}) weapon2->short(this_player());

    /* Some people will want to compare items with the same description. */
    if (str1 == str2)
    {
        if (swap)
        {
            str1 = "first " + str1;
            str2 = "second " + str2;
        }
        else
        {
            str2 = "first " + str2;
            str1 = "second " + str1;
        }
    }

    /* Gather the to-hit values. */
    stat1 = ({int}) weapon1->query_hit() + random(skill);
    stat2 = ({int}) weapon2->query_hit() + random(skill);

    if (stat1 > stat2)
    {
        stat1 = (100 - ((80 * stat2) / stat1));
        print1 = str1;
        print2 = str2;
    }
    else
    {
        stat1 = (100 - ((80 * stat1) / stat2));
        print1 = str2;
        print2 = str1;
    }

    stat1 = ((stat1 * sizeof(compare_strings[6])) / 100);
    write("Hitting someone with the " + print1 + " is " +
        compare_strings[6][((stat1 > 3) ? 3 : stat1)] + " the " +
        print2 + " and ");

    /* Compare the penetration values. */
    stat1 = ({int}) weapon1->query_pen() + random(skill);
    stat2 = ({int}) weapon2->query_pen() + random(skill);

    if (stat1 > stat2)
    {
        stat1 = (100 - ((80 * stat2) / stat1));
        print1 = str1;
        print2 = str2;
    }
    else
    {
        stat1 = (100 - ((80 * stat1) / stat2));
        print1 = str2;
        print2 = str1;
    }

    stat1 = ((stat1 * sizeof(compare_strings[7])) / 100);
    write("damage inflicted by the " + print1 + " is " +
        compare_strings[7][((stat1 > 3) ? 3 : stat1)] + " the " +
        print2 + ".\n");
}


/*
 * Function name: compare_armour
 * Description  : Compares the stats of two armour.
 * Arguments    : object armour1 - the left hand side to compare.
 *                object armour2 - the right hand side to compare.
 */
void compare_armour(object armour1, object armour2)
{
    int skill = (1000 / (1 + ({int}) this_player()->query_skill(SS_APPR_OBJ)));
    int seed  = to_int(OB_NUM(armour1)) + to_int(OB_NUM(armour2));
    int swap;
    int stat1;
    int stat2;
    string str1;
    string str2;
    string print1;
    string print2;
    object tmp;

    /* Always use the same order. After all, we don't want "compare X with Y"
     * to differ from "compare Y with X".
     */
    if (OB_NUM(armour1) > OB_NUM(armour2))
    {
        tmp = armour1;
        armour1 = armour2;
        armour2 = tmp;
	swap = 1;
    }

    str1 = ({string}) armour1->short(this_player());
    str2 = ({string}) armour2->short(this_player());

    /* Some people will want to compare items with the same description. */
    if (str1 == str2)
    {
        if (swap)
        {
            str1 = "first " + str1;
            str2 = "second " + str2;
        }
        else
        {
            str2 = "first " + str2;
            str1 = "second " + str1;
        }
    }

    /* Gather the armour class. */
    stat1 = ({int}) armour1->query_ac() + random(skill);
    stat2 = ({int}) armour2->query_ac() + random(skill);

    if (stat1 > stat2)
    {
        stat1 = (100 - ((80 * stat2) / stat1));
        print1 = str1;
        print2 = str2;
    }
    else
    {
        stat1 = (100 - ((80 * stat1) / stat2));
        print1 = str2;
        print2 = str1;
    }

    stat1 = ((stat1 * sizeof(compare_strings[8])) / 100);
    write("The " + print1 + " gives " +
        compare_strings[8][((stat1 > 3) ? 3 : stat1)] + " the " +
        print2 + ".\n");
}


int compare(string str)
{
    string str1;
    string str2;
    object *oblist;
    object obj1;
    object obj2;

    if (!sizeof(str) ||
        sscanf(str, "%s with %s", str1, str2) != 2)
    {
        notify_fail("Compare <whom/what> with <whom/what>?\n");
        return 0;
    }

    /* If we compare 'stats', then we are the left party. */
    if (str1 == "stats")
    {
        obj1 = this_player();
    }
    else
    /* Else, see the left hand side of whom/what we want to compare. */
    {
        oblist = parse_this(str1, "[the] %i");
        if (sizeof(oblist) != 1)
        {
            notify_fail("Compare <whom/what> with <whom/what>?\n");
            return 0;
        }
        obj1 = oblist[0];
    }

    if (str2 == "enemy")
    {
        object attacker = ({object}) obj1->query_attack();

        if (!objectp(attacker))
        {
            notify_fail(((obj1 == this_player()) ? "You are" :
                (({string}) obj1->query_The_name(this_player()) + " is")) +
                " not fighting anyone.\n");
            return 0;
        }

        if (!CAN_SEE_IN_ROOM(this_player()) || !CAN_SEE(this_player(), attacker))
        {
            notify_fail("You cannot see " + ((obj1 == this_player()) ? "your" :
                (({string}) obj1->query_the_name(this_player()) + "'s")) + " enemy.\n");
            return 0;
        }

        oblist = ({ attacker });
    }
    else
    /* Get the right hand side of what we want to compare. */
    {
        oblist = parse_this(str2, "[the] %i");
        if (sizeof(oblist) != 1)
        {
            notify_fail("Compare <whom/what> with <whom/what>?\n");
            return 0;
        }
    }
    obj2 = oblist[0];

    /* Yes, people will want to do the obvious. */
    if (obj1 == obj2)
    {
        notify_fail("It is pointless to compare something to itself.\n");
        return 0;
    }

    /* Compare the stats of two livings. */
    if (living(obj1))
    {
        if (!living(obj2))
        {
            if (obj1 == this_player())
            {
                notify_fail("You can only compare your stats to those of " +
                    "another living.\n");
                return 0;
            }
            notify_fail("The stats of " +
                ({string}) obj1->query_the_name(this_player()) +
                " can only be compared to those of another living.\n" );
            return 0;
        }

        compare_living(obj1, obj2);
        return 1;
    }

    str1 = function_exists("create_object", obj1);
    str2 = function_exists("create_object", obj2);

    /* Compare two weapons. */
    if (str1 == WEAPON_OBJECT)
    {
        if (str2 != WEAPON_OBJECT)
        {
            notify_fail("The " + ({string}) obj1->short(this_player()) +
                " can only be compared to another weapon.\n");
            return 0;
        }

        if (({int}) obj1->query_wt() != ({int}) obj2->query_wt())
        {
            notify_fail("The " + ({string}) obj1->short(this_player()) +
                " can only be compared to another weapon of the same " +
                "type.\n");
            return 0;
        }

        compare_weapon(obj1, obj2);
        return 1;
    }

    /* Compare two armours. */
    if (str1 == ARMOUR_OBJECT)
    {
        if (str2 != ARMOUR_OBJECT)
        {
            notify_fail("The " + ({string}) obj1->short(this_player()) +
                " can only be compared to another armour.\n");
            return 0;
        }

        if (({int}) obj1->query_at() != ({int}) obj2->query_at())
        {
            notify_fail("The " + ({string}) obj1->short(this_player()) +
                " can only be compared to another armour of the same " +
                "type.\n");
            return 0;
        }

        compare_armour(obj1, obj2);
        return 1;
    }

    notify_fail("It does not seem possible to compare " +
        LANG_THESHORT(obj1) + " with " + LANG_THESHORT(obj2) + ".\n");
    return 0;
}


/*
 * email - Display/change your email address.
 */
int email(string str)
{
    if (!stringp(str))
    {
        write("Your current email address is: " +
            ({string}) this_player()->query_mailaddr() + "\n");
        return 1;
    }

    this_player()->set_mailaddr(str);
    write("Changed your email address.\n");
    return 1;
}


/*
 * health - Display your health or that of someone else.
 */
int health(string str)
{
    object *oblist = ({ });
    int index;
    int size;
    int display_self;

    str = (stringp(str) ? str : "");

    switch(str)
    {
    case "":
        display_self = 1;
        break;

    case "enemy":
        object attacker = ({object}) this_player()->query_attack();

        if (!objectp(attacker))
        {
            notify_fail("You are not fighting anyone.\n");
            return 0;
        }


        if (!CAN_SEE_IN_ROOM(this_player()) ||
            !CAN_SEE(this_player(), attacker))
        {
            notify_fail("You cannot see your enemy.\n");
            return 0;
        }

        oblist = ({ attacker });
        break;

    case "team":
        oblist = ({object *}) this_player()->query_team_others();
        if (!sizeof(oblist))
        {
            str = "noteam";
            write("You are not in a team with anyone.\n");
        }

        if (!CAN_SEE_IN_ROOM(this_player()))
        {
            notify_fail("It is too dark to see.\n");
            return 0;
        }

        /* Only the team members in the environment of the player. */
        oblist &= all_inventory(environment(this_player()));

        oblist = FILTER_CAN_SEE(oblist, this_player());
        if (!sizeof(oblist) &&
            (str != "noteam"))
        {
            write("You cannot determine the health of any team member.\n");
        }

        display_self = 1;
        break;

    case "all":
        display_self = 1;
        /* Intentionally no "break". We need to catch "default" too. */

    default:
        oblist = parse_this(str, "[the] %l") - all_inventory(this_player());
        if (!display_self &&
            !sizeof(oblist))
        {
            notify_fail("Determine whose health?\n");
            return 0;
        }
    }

    if (display_self)
    {
        write("You are physically " +
            GET_NUM_DESC(({int}) this_player()->query_hp(), ({int}) this_player()->query_max_hp(), health_state) +
            " and mentally " +
            GET_NUM_DESC(({int}) this_player()->query_mana(), ({int}) this_player()->query_max_mana(), mana_state) +
            ".\n");
    }
    else if (!sizeof(oblist))
    {
        write("You can only determine the health of those you can see.\n");
        return 1;
    }

    index = -1;
    size = sizeof(oblist);
    while(++index < size)
    {
        write(({string}) oblist[index]->query_The_name(this_player()) + " is " +
            show_subloc_health(oblist[index], this_player()) + ".\n");
    }
    return 1;
}


/*
 * levels - Print a summary of different levels of different things
 */
public int levels(string str)
{
    string *ix, *levs;

    ix = sort_array(m_indices(lev_map), #'>);

    if (!str)
    {
        notify_fail("Available level descriptions:\n" +
                    COMPOSITE_WORDS(ix) + "." + "\n");
        return 0;
    }

    levs = lev_map[str];
    if (!sizeof(levs))
    {
        notify_fail("No such level descriptions. Available:\n" +
                    COMPOSITE_WORDS(ix) + "." + "\n");
        return 0;
    }

    write("Level descriptions for: " + capitalize(str) + "\n" +
          COMPOSITE_WORDS(levs) + "." + "\n");
    return 1;
}


/* **************************************************************************
 * options - Change/view the options
 */
nomask int options(string arg)
{
    string *args, rest;
    int     wi;

    if (!stringp(arg))
    {
        options("morelen");
        options("screenwidth");
        options("brief");
        options("echo");
        options("wimpy");
        options("see");
        options("unarmed");
        options("gagmisses");
        options("web");
//        options("merciful");
        options("autowrap");
        options("inventory");
	if (({int}) this_player()->query_wiz_level())
	{
	    options("autopwd");
	    options("autolinecmd");
	    options("timestamp");
	}
        return 1;
    }

    args = explode(arg, " ");
    if (sizeof(args) == 1)
    {
        switch(arg)
        {
        case "morelen":
        case "more":
            write("More length:     " +
                ({int}) this_player()->query_option(OPT_MORE_LEN) + "\n");
            break;

        case "screenwidth":
        case "sw":
            wi = ({int}) this_player()->query_option(OPT_SCREEN_WIDTH);
            write("Screen width:    " + ((wi > -1) ? ("" + wi) : "Off") + "\n");
            break;

        case "brief":
            write("Brief display:   " +
                (({int}) this_player()->query_option(OPT_BRIEF) ? "On" : "Off") + "\n");
            break;

        case "echo":
            write("Echo commands:   " +
                (({int}) this_player()->query_option(OPT_ECHO) ? "On" : "Off") + "\n");
            break;

        case "wimpy":
            wi = ({int}) this_player()->query_whimpy();
            write("Wimpy at:       '");
            if (wi)
            {
                wi = wi * sizeof(health_state) / 100;
                write(capitalize(health_state[wi]) + "'\n");
            }
            else
                write("Brave'\n");
            break;

        case "see":
        case "fights":
            write("See fights:      " +
                (({int}) this_player()->query_option(OPT_NO_FIGHTS) ?
		"Off" : "On") + "\n");
            break;

        case "unarmed":
            write("Unarmed combat:  " +
                (({int}) this_player()->query_option(OPT_UNARMED_OFF) ?
                "Off" : "On") + "\n");
            break;

        case "gagmisses":
            write("Gag misses:      " +
                (({int}) this_player()->query_option(OPT_GAG_MISSES) ?
                "On" : "Off") + "\n");
            break;

        case "merciful":
            write("Merciful combat: " +
                (({int}) this_player()->query_option(OPT_MERCIFUL_COMBAT) ?
                "On" : "Off") + "\n");
            break;

        case "autowrap":
            write("Auto-wrapping:   " +
                (({int}) this_player()->query_option(OPT_AUTOWRAP) ?
                "On" : "Off") + "\n");
            break;

        case "web":
            write("Web publication: " +
                (({int}) this_player()->query_option(OPT_WEBPERM) ?
                "No" : "Yes") + "\n");
            break;

        case "inventory":
        case "table":
            write("Table inventory: " +
                (({int}) this_player()->query_option(OPT_TABLE_INVENTORY) ?
                "On" : "Off") + "\n");
            break;

        case "autopwd":
	case "pwd":
	    if (({int}) this_player()->query_wiz_level())
	    {
	        write("Auto pwd on cd:  " +
		    (({int}) this_player()->query_option(OPT_AUTO_PWD) ?
		    "On" : "Off") + "\n");
		break;
	    }
	    /* Intentional fallthrough to default if not a wizard. */

	case "autolinecmd":
	case "cmd":
	    if (({int}) this_player()->query_wiz_level())
	    {
		write("Auto line cmds:  " +
		    (({int}) this_player()->query_option(OPT_AUTOLINECMD) ?
		    "On" : "Off") + "\n");
		break;
	    }
	    /* Intentional fallthrough to default if not a wizard. */

	case "timestamp":
	    if (({int}) this_player()->query_wiz_level())
	    {
		write("Timestamp lines: " +
		    (({int}) this_player()->query_option(OPT_TIMESTAMP) ?
		    "On" : "Off") + "\n");
		break;
	    }
	    /* Intentional fallthrough to default if not a wizard. */

	default:
            return notify_fail("Syntax error: No such option.\n");
            break;
        }
        return 1;
    }

    rest = implode(args[1..], " ");

    switch(args[0])
    {
    case "morelen":
    case "more":
        if (!({int}) this_player()->set_option(OPT_MORE_LEN, to_int(args[1])))
        {
            notify_fail("Syntax error: More length must be in the range of " +
                "1 - 100.\n");
            return 0;
        }
        options("morelen");
        break;

    case "screenwidth":
    case "sw":
        if (args[1] == "off")
        {
            if (!({int}) this_player()->set_option(OPT_SCREEN_WIDTH, -1))
            {
                notify_fail("Failed to reset screen width setting. Please " +
                    "make a sysbug report.\n");
                return 0;
            }
        }
        else if (!({int}) this_player()->set_option(OPT_SCREEN_WIDTH, to_int(args[1])))
        {
            notify_fail("Syntax error: Screen width must be in the range " +
                "40 - 200 or 'off' to disable wrapping by the game.\n");
            return 0;
        }
        options("screenwidth");
        break;

    case "brief":
        this_player()->set_option(OPT_BRIEF, (args[1] == "on"));
        options("brief");
        break;

    case "echo":
        this_player()->set_option(OPT_ECHO, (args[1] == "on"));
        options("echo");
        break;

    case "wimpy":
        if (args[1] == "brave")
        {
            this_player()->set_whimpy(0);
        }
        else if (args[1] == "?")
            write("brave, " + implode(health_state, ", ") + "\n");
        else
        {
            wi = member(rest, health_state);
            if (wi < 0)
            {
                notify_fail("No such health descriptions (" + rest +
                    ") Available:\n" +
                    COMPOSITE_WORDS(health_state) + ".\n");
                return 0;
            }

            wi = (100 * (wi + 1)) / sizeof(health_state);
            if (wi > 99)
                wi = 99;

            this_player()->set_whimpy(wi);
        }
        options("wimpy");
        break;

    case "see":
    case "fights":
        this_player()->set_option(OPT_NO_FIGHTS, (args[sizeof(args) - 1] != "on"));
        options("see");
        break;

    case "unarmed":
        /* This to accomodate people typing "options unarmed combat" */
        args -= ({ "combat" });
        if (sizeof(args) == 2)
        {
            this_player()->set_option(OPT_UNARMED_OFF, (args[1] != "on"));
            this_player()->update_procuse();
        }
        options("unarmed");
        break;

    case "gagmisses":
        this_player()->set_option(OPT_GAG_MISSES, (args[1] == "on"));
        options("gagmisses");
        break;

    case "merciful":
        this_player()->set_option(OPT_MERCIFUL_COMBAT, (args[1] == "on"));
        options("merciful");
        break;

    case "autowrap":
        this_player()->set_option(OPT_AUTOWRAP, (args[1] == "on"));
        options("autowrap");
        break;

    case "web":
        this_player()->set_option(OPT_WEBPERM, (args[1] == "no"));
        options("web");
        break;

    case "inventory":
    case "table":
        this_player()->set_option(OPT_TABLE_INVENTORY, (args[1] == "on"));
        options("inventory");
        break;

    case "autopwd":
    case "pwd":
        if (({int}) this_player()->query_wiz_level())
	{
            this_player()->set_option(OPT_AUTO_PWD, (args[1] == "on"));
            options("autopwd");
	    break;
	}
        /* Intentional fallthrough to default if not a wizard. */

    case "autolinecmd":
    case "cmd":
        if (({int}) this_player()->query_wiz_level())
	{
            this_player()->set_option(OPT_AUTOLINECMD,(args[1] == "on"));
	    // Make sure the line command set gets updated.
	    "/cmd/wiz/apprentice"->update_commands();
            options("autolinecmd");
	    break;
	}
        /* Intentional fallthrough to default if not a wizard. */

    case "timestamp":
        if (({int}) this_player()->query_wiz_level())
	{
            this_player()->set_option(OPT_TIMESTAMP, (args[1] == "on"));
            options("timestamp");
	    break;
	}
        /* Intentional fallthrough to default if not a wizard. */

    default:
        return notify_fail("Syntax error: No such option.\n");
        break;
    }
    return 1;
}


/*
 * vitals - Give vital state information about the living.
 */
varargs int vitals(string str, object target = this_player())
{
    string name;
    int self;
    int value1;
    int value2;

    if (!sizeof(str))
    {
        str = "all";
    }

    self = (target == this_player());
    name = capitalize(({string}) target->query_real_name());
    switch(str)
    {
    case "age":
        write((self ? "You are" : (name + " is")) + " " +
            CONVTIME(({int}) target->query_age() * 2) + " of age.\n");
        return 1;

    case "align":
    case "alignment":
        write((self ? "You are" : (name + " is")) + " " +
            ({string}) target->query_align_text() + ".\n");
        return 1;

    case "all":
        vitals("health", this_player());
        vitals("panic", this_player());
        vitals("stuffed", this_player());
        vitals("intox", this_player());
        vitals("alignment", this_player());
        vitals("encumbrance", this_player());
        vitals("age", this_player());
        return 1;

    case "encumbrance":
        write((self ? "You are" : (name + " is")) + " " +
            GET_PROC_DESC(target->query_encumberance_weight(), enc_weight) + ".\n");
        return 1;

    case "health":
    case "mana":
        write((self ? "You are" : (name + " is")) + " physically " +
            GET_NUM_DESC(target->query_hp(), target->query_max_hp(), health_state) +
            " and mentally " +
            GET_NUM_DESC(target->query_mana(), target->query_max_mana(), mana_state) +
            ".\n");
        return 1;

    case "intox":
    case "intoxication":
        if (({int}) target->query_intoxicated())
        {
            write((self ? "You are" : (name + " is")) + " " +
                GET_NUM_DESC_SUB(({int}) target->query_intoxicated(),
                    ({int}) target->query_prop(LIVE_I_MAX_INTOX), intox_state,
                    SD_STAT_DENOM, 0) + ".\n");
        }
        else
        {
            write((self ? "You are" : (name + " is")) + " sober.\n");
        }
        return 1;

    case "mail":
        value1 = ({int}) MAIL_CHECKER->query_mail(target);
        write((self ? "You have " : (name + " has ")) + MAIL_FLAGS[value1] + ".\n");
        return 1;

    case "panic":
    case "fatigue":
        /* Current fatigue really is an "energy left" value that counts down. */
        value1 = ({int}) target->query_max_fatigue() - ({int}) target->query_fatigue();
        write((self ? "You feel" : (name + " feels")) + " " +
            GET_NUM_DESC_SUB(({int}) target->query_panic(), F_PANIC_WIMP_LEVEL(({int}) target->query_stat(SS_DIS)), panic_state, SD_STAT_DENOM, 2) +
            " and " +
            GET_NUM_DESC_SUB(value1, ({int}) target->query_max_fatigue(), fatigue_state, SD_STAT_DENOM, 1) + ".\n");
        return 1;

    case "stuffed":
    case "soaked":
        write((self ? "You can" : (name + " can")) + " " +
            GET_NUM_DESC(target->query_stuffed(), ({int}) target->query_prop(LIVE_I_MAX_EAT), stuff_state) +
            " and " +
            GET_NUM_DESC(target->query_soaked(), ({int}) target->query_prop(LIVE_I_MAX_DRINK), soak_state) + ".\n");
        return 1;

    default:
        if (!({int}) this_player()->query_wiz_level())
        {
            notify_fail("The argument " + str + " is not valid for vitals.\n");
            return 0;
        }
        if (!objectp(target = find_player(lower_case(str))))
        {
            notify_fail("There is no player " + capitalize(str) +
                " in the realms at present.\n");
            return 0;
        }
        vitals("health", target);
        vitals("panic", target);
        vitals("stuffed", target);
        vitals("intox", target);
        vitals("alignment", target);
        vitals("encumbrance", target);
        vitals("age", target);
        return 1;
    }

    write("Impossible end of vitals. Please make a sysbug report of this.\n");
    return 1;
}


/*
 * Function name: show_stats
 * Description:   Gives information on the stats
 */
varargs int show_stats(string str)
{
    int a, i, j, c;
    object ob;
    string start_be, start_have, *stats;
    string orig_brute, actual_brute;

    if (str == "reset")
    {
        this_player()->add_prop(PLAYER_I_LASTXP, ({int}) this_player()->query_exp());
        write("Resetting your progress counter.\n");
        return show_stats(0);
    }

    if (!sizeof(str))
    {
        ob = this_player();
        start_be = "You are ";
        start_have = "You have ";
    }
    else
    {
        if(!((ob = find_player(str)) && ({int}) this_player()->query_wiz_level()))
        {
            notify_fail("Curious aren't we?\n");
            return 0;
        }
        start_be = capitalize(str) + " is ";
        start_have = capitalize(str) + " has ";
    }

    a = ({int}) ob->query_prop(PLAYER_I_LASTXP);
    j = ({int}) ob->query_exp() - a;
    if (a <= 0)
    {
        write("Your progress indicator was not working properly " +
            "and is now reset.\n");
        ob->add_prop(PLAYER_I_LASTXP, ({int}) ob->query_exp());
    }
    else if (j > 0)
    {
        /* The progress is measured relatively to your current exp. If
         * you gained more than 6.6% of the exp you had when you logged
         * in, you get the maximum progress measure. There are a minimum
         * and a maximum, though.
         */
        a /= 15;
        if (a > SD_IMPROVE_MAX)
            a = SD_IMPROVE_MAX;
        else if (a < SD_IMPROVE_MIN)
            a = SD_IMPROVE_MIN;

        write(start_have + "made " + GET_NUM_DESC(j, a, improve_fact) +
            " progress since you last logged in.\n");
    }
    else
    {
        write(start_have + "made no measurable progress since you logged " +
            "in today.\n");
    }

    stats = ({ });
    for (i = 0; i < SS_NO_EXP_STATS; i++)
    {
        stats += ({ GET_STAT_LEVEL_DESC(i, ({string}) ob->query_stat(i)) });
    }
    write(start_be + LANG_ADDART(COMPOSITE_WORDS(stats)) +  " " + ({string}) ob->query_nonmet_name() + ".\n");

    /* brutalfactor */
    actual_brute = GET_NUM_DESC_SUB(to_int(({int}) ob->query_brute_factor(0) * 1000.0), 1000, brute_fact, SD_STAT_DENOM, 2);
    orig_brute = GET_NUM_DESC_SUB(to_int(({int}) ob->query_brute_factor(1) * 1000.0), 1000, brute_fact, SD_STAT_DENOM, 2);

    write(start_be + actual_brute + ".\n");
    if (actual_brute != orig_brute)
    {
        write("After recovery from death, " + lower_case(start_be) +
	    orig_brute + ".\n");
    }

    return 1;
}


/*
 * skills - Give information on a livings skills
 */
/*
 * Function name: show_skills
 * Description:   Gives information on the stats
 */
varargs int show_skills(string str)
{
    int index, wrap, iLow, iHigh, num;
    object player = this_player();
    int *skills;
    string *words;
    mapping skdesc;
    string group = "";

    wrap = 1;
    skdesc = SS_SKILL_DESC;

    words = str ? explode(str, " ") : ({ });
    switch(sizeof(words))
    {
    case 0:
        /* Default situation. Player wants to see all his stats. */
        break;
    case 1:
        /* Person wants to see a group of himself, or see someone elses stats. */
        if (!({int}) this_player()->query_wiz_level() || !(player = find_player(words[0])))
        {
            player = this_player();
            group = words[0];
        }
        break;
    case 2:
        /* Player specifies both the person to see and the group to see. */
        if (!({int}) this_player()->query_wiz_level())
            player = find_player(words[0]);
        group = words[1];
        break;
    default:
        notify_fail("Too many arguments. Syntax: stats [player] [skill group]\n");
        return 0;
    }

    if (!objectp(player))
    {
        notify_fail("No player " + words[0] + " found.\n");
        return 0;
    }

    SKILL_LIBRARY->sk_init();

    switch (group)
    {
        case "general":
            iLow = 70;
            iHigh = 200;
            break;
        case "fighting":
            iLow = 0;
            iHigh = 29;
            break;
        case "magic":
            iLow = 30;
            iHigh = 49;
            break;
        case "thief":
            iLow = 50;
            iHigh = 69;
            break;
        case "guild":
            iLow = 100000;
            iHigh = MAXINT;
            break;
        case "all":
        case "":
            iLow = 0;
            iHigh = MAXINT;
            break;
        default:
            notify_fail("Unknown group '" + group + "'.\n" +
                "Valid groups are: general, fighting, magic, thief, guild or all.\n");
            return 0;
    }

    skills = ({int *}) player->query_all_skill_types();

    foreach(int skill: skills)
    {
        if (skill < iLow || skill > iHigh)
            continue;

        if (!(num = ({int}) player->query_skill(skill)))
        {
            player->remove_skill(skill);
            continue;
        }
        if (pointerp(skdesc[skill]))
            str = skdesc[skill][0];
        else if (!sizeof(str = ({string}) player->query_skill_name(skill)))
            continue;

        /* Print the text in two columns. */
        if (++wrap % 2)
        {
            write(sprintf("%-18s %s\n", str + ":", ({string}) SKILL_LIBRARY->sk_rank(num)));
        }
        else
        {
            write(sprintf("%-18s %-20s ", str + ":", ({string}) SKILL_LIBRARY->sk_rank(num)));
        }
    }
    if (wrap > 1)
        write("\n");
    else
    {
        write(((player == this_player()) ? "You have " :
            (({string}) player->query_name() + " has ")) + "no skills" +
            (sizeof(group) ? " in the " + group + " group" : "") + ".\n");
    }

    return 1;
}
