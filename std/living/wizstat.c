/*
 * /std/living/wizstat.c
 *
 * Contains the code to implement the various wizardly information commands.
 */

#include <filepath.h>
#include <formulas.h>

public int base_stat(int acc_exp)
{
    return F_EXP_TO_STAT(acc_exp);
}

public string round_stat(int stat)
{
    if (stat > 100000)
	return sprintf("%3.1fM", to_float(stat) / 1000000.0);
    else if (stat > 1000)
	return sprintf("%3.1fk", to_float(stat) / 1000.0);

    return sprintf("%d", stat);
}

/*
 * Function name: stat_living
 * Description  : Give status information on this living.
 * Returns      : string - the description.
 */
public string stat_living()
{
    string str, tmp;
    object to;
    int *stats = ({ SS_STR, SS_DEX, SS_CON, SS_INT, SS_WIS, SS_DIS,
                    SS_RACE, SS_OCCUP, SS_LAYMAN, SS_CRAFT });

    to = this_object();
    str = sprintf(
	  "Name: %-11s Rank: %-10s " +
#ifdef USE_WIZ_LEVELS
          "(%-2d) " +
#endif
	  "Gender: %-10s Race: %s (%s)\n" +
          "File: %-35s  Uid&Euid: %s\n"  +
	  "-----------------------------------------------------------------------------\n" +
	  "Exp: %9d %8s)  Quest: %7d  Combat: %8d  General: %8d\n" +
 	  "Weight: %6d %8s)  Volume: %6d %8s)\n" +
	  "Hp:  %4d %5s)  Mana:  %4d %5s)  Panic: %4d %5s)  Fatigue: %4d %5s)\n" +
	  "Eat: %4d %5s)  Drink: %4d %5s)  Intox: %4d %5s)  Av.Stat: %4d\n" +
 	  "\n" +
	  "Stat: %@7s\n"  +
          "Value:%@7d\n" +
          "Base: %@7d\n" +
          "Exp:  %@7s\n" +
	  "Learn:%@7d\n\n" +
	  "Align: %d  Scar: %d  Hide: %d  Invis: %d  Ghost: %d  Npc: %d  Whimpy: %d%%\n",
		  capitalize(query_real_name()),
		  WIZ_RANK_NAME(({int}) SECURITY->query_wiz_rank(query_real_name())),
#ifdef USE_WIZ_LEVELS
		  ({int}) SECURITY->query_wiz_level(query_real_name()),
#endif
		  ({string}) to->query_gender_string(),
		  ({string}) to->query_race_name(),
		  ({string}) to->query_race(),
		  RPATH(object_name(this_object())[..34]),
		  getuid(this_object()) + ":" + geteuid(this_object()),
		  ({int}) to->query_exp(),
		  ((({int}) to->query_max_exp() > ({int}) to->query_exp()) ? "(" + ({int}) to->query_max_exp() : "(max"),
		  ({int}) to->query_exp_quest(),
		  ({int}) to->query_exp_combat(),
		  ({int}) to->query_exp_general(),
		  ({int}) to->query_prop(OBJ_I_WEIGHT),
		  "(" + ({int}) to->query_prop(CONT_I_MAX_WEIGHT),
		  ({int}) to->query_prop(OBJ_I_VOLUME),
		  "(" + ({int}) to->query_prop(CONT_I_MAX_VOLUME),
		  ({int}) to->query_hp(),
		  "(" + ({int}) to->query_max_hp(),
		  ({int}) to->query_mana(),
		  "(" + ({int}) to->query_max_mana(),
		  ({int}) to->query_panic(),
		  "(" + F_PANIC_WIMP_LEVEL(({int}) to->query_stat(SS_DIS)),
		  ({int}) to->query_fatigue(),
		  "(" + ({int}) to->query_max_fatigue(),
		  ({int}) to->query_stuffed(),
		  "(" + ({int}) to->query_prop(LIVE_I_MAX_EAT),
		  ({int}) to->query_soaked(),
		  "(" + ({int}) to->query_prop(LIVE_I_MAX_DRINK),
		  ({int}) to->query_intoxicated(),
		  "(" + ({int}) to->query_prop(LIVE_I_MAX_INTOX),
		  ({int}) to->query_average_stat(),
		  SS_STAT_DESC,
                  map(stats, (: ({int}) to->query_stat() :)),
                  map(map(stats, (: ({int}) to->query_acc_exp($1) :)), #'base_stat),
                  map(map(stats, (: ({int}) to->query_acc_exp($1) :)), #'round_stat),
		  ({int}) to->query_learn_pref(-1),
		  ({int}) to->query_alignment(),
		  ({int}) to->query_scar(),
		  ({int}) to->query_prop(OBJ_I_HIDE),
		  ({int}) to->query_invis(),
		  ({int}) to->query_ghost(),
		  ({int}) to->query_npc(),
		  ({int}) to->query_whimpy());

    if (sizeof(tmp = ({string}) to->query_prop(OBJ_S_WIZINFO)))
	str += "Wizinfo:\n" + tmp;

    return str;
}

/*
 * Function name: fix_skill_desc
 * Description  : This function will compose the string describing the
 *                individual skills the player has.
 * Arguments    : int sk_type     - the skill number.
 *                mapping sk_desc - the mapping describing the skills.
 * Returns      : string - the description for this skill.
 */
nomask static string fix_skill_desc(int sk_type, mapping sk_desc)
{
    string desc;

    if (pointerp(sk_desc[sk_type]))
    {
        desc = sk_desc[sk_type][0];
    }
    else
    {
        if (!(desc = ({string}) this_object()->query_skill_name(sk_type)))
	{
	    desc = "special";
	}
    }

    return sprintf("%s: %3d (%6d)", desc[..23],
		({int}) this_object()->query_skill(sk_type), sk_type);
}

/*
 * Function name: skill_living
 * Description  : This function returns a proper string describing the
 *                skills of this living.
 * Returns      : string - the description.
 */
public string skill_living()
{
    string *skills;
    string sk;
    int *sk_types;
    int index;
    int size;
    mapping sk_desc;

    sk_types = sort_array(query_all_skill_types(), #'>);

    if (!sizeof(sk_types))
    {
	return capitalize(query_real_name()) + " has no skills.\n";
    }

    sk_desc = SS_SKILL_DESC;
    sk = "";
    skills = map(sk_types, (: fix_skill_desc($1, sk_desc) :));
    size = ((sizeof(skills) + 1) / 2);
    skills += ({ "" });
    index = -1;
    while(++index < size)
    {
	sk += sprintf("%38s %38s\n", skills[index], skills[index + size]);
    }

    return "Skills of " + capitalize(query_real_name()) + ":\n" + sk;
}
