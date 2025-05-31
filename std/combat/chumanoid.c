/*
  /std/combat/chumanoid.c

  This is the externalized combat routines for humanoids.

  This combat object predefines a set of attacks and hitlocations
  for humanoid living objects. It also keeps track of the total percentage
  of attacks that can be made each turn and distributes those percentages
  over the attacks depending on effectiveness of weapon wielded.

  The distribution formula is:

          %use = %maxuse * (wchit*wcpen) / ( sum(wchit*wcpen) )

  This formula is used if the 'set_attuse()' function is called and the
  value set to something different than 0. Otherwise the %use is left
  unaffected.

  This object is cloned and linked to a specific individual when
  engaged in combat. The actual object resides in 'limbo'.
*/


#pragma strict_types

inherit "/std/combat/ctool";

#include <wa_types.h>
#include "combat.h"
#include <formulas.h>
#include <ss_types.h>
#include <std.h>
#include <options.h>
#include <libfiles.h>

public void cb_modify_procuse();

static  int             attuse;      /* Total %use, 100% is 1 attack */


/*
* Function name: create_ctool
* Description:   Reset the combat functions
*/
public nomask void create_ctool()
{
    if (me)
    {
	return;
    }

    this_object()->create_chumanoid();
}


/*
 * Function name: cb_configure
 * Description:   Configure humanoid attacks and hitlocations.
 * Returns:       True if hit, otherwise 0.
 */
public void cb_configure()
{
    ::cb_configure();

    add_attack(0, 0, 0, 0, W_RIGHT); me->cr_reset_attack(W_RIGHT);
    add_attack(0, 0, 0, 0, W_LEFT);  me->cr_reset_attack(W_LEFT);
    add_attack(0, 0, 0, 0, W_BOTH);  me->cr_reset_attack(W_BOTH);
    add_attack(0, 0, 0, 0, W_FOOTR); me->cr_reset_attack(W_FOOTR);
    add_attack(0, 0, 0, 0, W_FOOTL); me->cr_reset_attack(W_FOOTL);
    cb_modify_procuse();

    add_hitloc(0, 0, 0, A_HEAD);  me->cr_reset_hitloc(A_HEAD);
    add_hitloc(0, 0, 0, A_L_ARM); me->cr_reset_hitloc(A_L_ARM);
    add_hitloc(0, 0, 0, A_R_ARM); me->cr_reset_hitloc(A_R_ARM);
    add_hitloc(0, 0, 0, A_TORSO); me->cr_reset_hitloc(A_TORSO);
    add_hitloc(0, 0, 0, A_LEGS);  me->cr_reset_hitloc(A_LEGS);

    map(({object *}) qme()->query_weapon(-1), #'cb_wield_weapon);
    map(({object *}) qme()->query_armour(-1), #'cb_wear_arm);
}


/*
 * Description: Humanoids might reallocate what attacks they use when the
 *              attacks are modified. (If maxuse is set)
 *              The distribution formula is:
 *
 *                       %use = %maxuse * (wchit*wcpen) / ( sum(wchit*wcpen) )
 */
public void cb_modify_procuse()
{
    int swc = 0, puse, weapon_no, unarmed_off;
    object tool;
    int *attid;
    mixed *attack;

    if (!attuse)
        return;

    attid = query_attack_id();
    weapon_no = sizeof(cb_query_weapons());
    unarmed_off = ({int}) me->query_option(OPT_UNARMED_OFF);

    mapping enabled_attacks = ([]);
    mapping attacks_data = ([]);

    foreach (int id in attid)
    {
        attack = query_attack(id);
        attacks_data[id] = attack;
        if (!attack[ATT_OBJ])
        {
            if (unarmed_off && (weapon_no > 0))
                continue;
            if ((tool = ({object})qme()->query_tool(id)) &&
                ({int})tool->query_attack_blocked(id))
                continue;
        }
        enabled_attacks[id] = 1;
        swc += attack[ATT_WCHIT] * F_PENMOD(attack[ATT_WCHIT], attack[ATT_SKILL]);
    }

    foreach (int id in attid)
    {
        attack = attacks_data[id];
        if (swc && enabled_attacks[id])
        {
            puse = (attuse * attack[ATT_WCHIT] *
                    F_PENMOD(attack[ATT_WCHIT], attack[ATT_SKILL])) / swc;
        }
        else
        {
            puse = 0;
        }
        ::add_attack(attack[ATT_WCHIT], attack[ATT_WCPEN],
                     attack[ATT_DAMT], puse, id,
                     (attack[ATT_SKILL] ? attack[ATT_SKILL] : -1),
                     attack[ATT_OBJ]);
    }
}


/*
 * Description: Set the %attacks used each turn. 100% is one attack / turn
 * Arguments:   sumproc: %attack used
 */
public void cb_set_attackuse(int sumproc)
{
    log_debug("Setting attackuse to %d", sumproc);
    attuse = sumproc;
    cb_modify_procuse();
}


/*
 * Description: Query the total %attacks used each turn. 100% is one attack / turn
 * Returns:     The attackuse
 */
public int
cb_query_attackuse()
{
    return attuse;
}


/*
 * Description: Add an attack, see /std/combat/cbase.c
 */
varargs int add_attack(int wchit, mixed wcpen, int damtype, int prcuse, int id, int skill,
    object wep)
{
    int ret;
    log_debug("Adding attack in chumanoid: %d", id);

    ret = ::add_attack(wchit, wcpen, damtype, prcuse, id, skill, wep);

    return ret;
}


/*
 * Function name: cb_wield_weapon
 * Description:   Wield a weapon.
 * Arguments:	  wep - The weapon to wield.
 * Returns:       string - error message (weapon not wielded)
 *                1 - success (weapon wielded)
 */
public mixed cb_wield_weapon(object wep)
{
    int aid, wcskill, owchit, owcpen;
    mixed *att;
    mixed str;

    if (!({int}) me->query_wiz_level() && function_exists("create_object", wep) != WEAPON_OBJECT)
    {
    	return "The " + ({string}) wep->short() + " is not a true weapon!\n";
    }

    if (stringp(str = ::cb_wield_weapon(wep)))
    {
    	return str;
    }

    aid = ({int}) wep->query_attack_id();

    if (cb_query_weapon(aid) == wep)
    {
    	att = query_attack(aid);
    	/*
         * We get no more use of the weapon than our skill with it allows.
    	 */
        wcskill = ({int}) me->query_skill(SS_WEP_FIRST + (({int}) wep->query_wt() - W_FIRST));

        if (wcskill < 1)
            wcskill = -1;

        add_attack(att[0], att[1], att[2], att[3], aid, wcskill, wep);
        cb_modify_procuse();
    }

    return 1;
}


/*
 * Function name: cb_wear_arm
 * Description:   Wear an armour
 * Arguments:	  arm - The armour.
 * Returns:       True if worn, errtext if fail
 */
public mixed cb_wear_arm(object arm)
{
    if (!({int}) me->query_wiz_level() && function_exists("create_object", arm) != ARMOUR_OBJECT)
    {
    	return "The " + ({string}) arm->short() + " is not a true armour!\n";
    }

    return ::cb_wear_arm(arm);
}
