/*
 * /std/weapon.c
 *
 * Contains all routines relating to weapons of any kind.
 * Defined functions and variables:
 *
 * set_hit(int)         Sets the weapon "to hit" value.
 *
 * set_pen(int)         Sets the weapon penetration value.
 *
 * set_pm(int *)        Sets some modifiers for the penetration on the weapon.
 *                      only useful if weapon is of multiple damage type.
 *
 * set_wt(type)         Set the weapon type.
 *                      If not defined, set to default.
 *
 * set_dt(type)         Set the damage type.
 *                      If not defined, set to default.
 *
 * set_hands(which)     Set the hand(s) used to wield the weapon.
 *                      If not defined, set to default.
 *
 * set_ac_mod(int)      Sets the armour class modifier for the weapon.
 *
 * set_wf(obj)          Sets the name of the object that contains the function
 *                      to call for extra defined wield() and unwield()
 *                      functions.
 *
 * query_wielded()      Returns the wielder if this object is wielded
 *
 * Weapons recover by default.
 */


#pragma strict_types

inherit "/std/object";

#include <cmdparse.h>
#include <files.h>
#include <formulas.h>
#include <language.h>
#include <macros.h>
#include <ss_types.h>
#include <stdproperties.h>
#include <wa_types.h>

static int      wep_hit,        /* Weapon to hit */
                wep_pen,        /* Weapon penetration */
                wep_wt,         /* Weapon type */
                wep_dt,         /* Damage type */
                wep_hands,      /* How many hands the weapon takes */
                wielded,        /* Wielded or not */
                wielded_in_hand,/* Wielded in which hand */
                *m_pen,         /* Modifiers for the wep_pen */
                ac_modifier,    /* Modifier for the armour class */
                hits,           /* No of hits the weapon have made */
                dull,           /* How dull the weapon has become */
                corroded,       /* Corrotion on the weapon */
                repair_dull,    /* How much dullness we have repaired */
                repair_corr,    /* How much corrosion we have repaired */
                likely_corr,    /* How likely will this weapon corrode */
                likely_break,   /* How likely will this weapon break? */
                likely_dull,    /* How likely will it be dulled by battle? */
                max_value;      /* Value of weapon at prime condition */
static object   wielder,        /* Who is holding it */
                wield_func;     /* Object that defines extra wield/unwield */
static private int      will_not_recover;       /* True if it won't recover */

/*
 * Prototypes
 */
        string  wep_condition_desc();
nomask  void update_prop_settings();
        int     query_value();
varargs void remove_broken(int silent);


/*
 * Function name: create_weapon
 * Description  : Create the weapon. You must define this function to
 *                construct the weapon.
 */
public void create_weapon()
{
}

/*
 * Function name: create
 * Description  : Create the weapon. As this function is declared nomask
 *                you must use the function create_weapon to actually
 *                construct it. This function does some basic initializing.
 */
public nomask void create_object()
{
    // Set default values for weapon attributes
    wep_wt = F_WEAPON_DEFAULT_WT;
    wep_dt = F_WEAPON_DEFAULT_DT;
    wep_hands = F_WEAPON_DEFAULT_HANDS;
    
    // Set default weapon characteristics
    likely_dull = 10;
    likely_corr = 10;
    likely_break = 10;
    
    // Initialize basic object properties
    set_name("weapon");
    set_pname("weapons");
    add_adj("unwielded");
    wielded = 0;
    
    // Set default object properties
    add_prop(OBJ_I_VALUE, #'query_value);
    add_prop(OBJ_I_VOLUME, 500);
    add_prop(OBJ_I_WEIGHT, 200);
    
    // Initialize penetration modifiers
    m_pen = ({ 0, 0, 0 });
    ac_modifier = 0;

    /* Determine if this weapon will recover on reboot
     * Will be set to true if the weapon should NOT recover
     */
    will_not_recover = (random(100) < PERCENTAGE_OF_RECOVERY_LOST);

    // Call weapon-specific creation function
    create_weapon();

    // Update properties based on weapon configuration
    update_prop_settings();
}


/*
 * Function name: reset_weapon
 * Description  : This function can be used to reset the weapon at a regular
 *                interval. However, if you do so, you must also call the
 *                function enable_reset() from your create_weapon() function
 *                in order to start up the reset proces.
 */
public void reset_weapon()
{
}


/*
 * Function name: reset_object
 * Description  : Reset the weapon. If you want to make the weapon reset
 *                at a certain interval you must use reset_weapon as this
 *                function is nomasked.
 */
public nomask void reset_object()
{
    reset_weapon();
}


/*
 * Function name: short
 * Description  : The short description. We modify it when the weapon is
 *                broken to prepend "broken" to the description.
 * Arguments    : object for_obj - the object that wants to know.
 * Returns      : string - the description.
 */
public varargs string short(object for_obj)
{
    string desc = ::short(for_obj);
    
    if (query_prop(OBJ_I_BROKEN))
    {
        return "broken " + desc;
    }

    return desc;
}


/*
 * Function name: plural_short
 * Description  : The plural short description. When the weapon is broken,
 *                we alter it to prepend "broken" to the description.
 * Arguments    : object for_obj - the object that wants to know.
 * Returns      : string - the description.
 */
public varargs string plural_short(object for_obj)
{
    string str = ::plural_short(for_obj);

    /* We make this additional check for stringp(str) because if no plural
     * short has been set, we shouldn't alter it. The plural short will
     * be generated if no plural short has been set.
     */
    if (query_prop(OBJ_I_BROKEN) && stringp(str))
    {
        return "broken " + str;
    }

    return str;
}


/*
 * Function name: long
 * Description  : The long description. We add the information about the
 *                condition to it.
 * Arguments    : string str - a possible add-item to look for.
 *                object for_obj - the object that wants to know.
 * Returns      : string - the long description.
 */
public varargs string long(string str, object for_obj)
{
    return ::long(str, for_obj) + (str ? "" : wep_condition_desc());
}


/*
 * Function name: wield_me
 * Description  : When the player tries to wield this weapon, this function
 *                is called. If the player managed to wield the weapon,
 *                the message is printed in this routine. Error messages are
 *                returned.
 * Returns      : int 1  - success
 *                string - a fail message (nothing is printed).
 */
nomask mixed wield_me()
{
    int wret;
    string wfail;
    
    // Early validation checks
    if (wielded)
        return "You already wield " + LANG_THESHORT(this_object()) + ".\n";
    
    if (this_player() != environment(this_object()))
        return "You must carry the " + short() + " to be able to wield it.\n";
    
    if (query_prop(OBJ_I_BROKEN))
        return "The " + short() + " is broken and not wieldable.\n";

    wielder = this_player();

    // Check for an available hand to wield the weapon in
    wielded_in_hand = wep_hands;

    if (wep_hands != W_ANYH)
    {
        // Check if both hands are occupied
        if (({string}) wielder->query_tool(W_BOTH) && wep_hands < W_FOOTR)
        {
            return "Your hands seem busy with other things.\n";
        }

        // Check if the specific hand is occupied
        if (({string}) wielder->query_tool(wep_hands))
        {
            return "The " + ({string}) (({object})wielder->query_tool(wep_hands))->short() + " is in the way.\n";
        }

        // Check if two-handed weapon but one hand is occupied
        if ((wep_hands == W_BOTH) &&
            (({string}) wielder->query_tool(W_RIGHT) || ({string}) wielder->query_tool(W_LEFT)))
        {
            return "You need both hands to wield it.\n";
        }
    }
    else if (!({string}) wielder->query_tool(W_BOTH) && !({string}) wielder->query_tool(W_RIGHT))
    {
        wielded_in_hand = W_RIGHT;
    }
    else if (!({string}) wielder->query_tool(W_BOTH) && !({string}) wielder->query_tool(W_LEFT))
    {
        wielded_in_hand = W_LEFT;
    }
    else
    {
        return "You have no hand left to wield that weapon.\n";
    }

    // Try to wield the weapon - check if wielder allows it
    if (stringp(wfail = ({mixed}) wielder->wield(this_object())))
    {
        return wfail;
    }

    wret = 0;

    // Check wield function in another object if it exists
    if (wield_func && (wret = ({mixed}) wield_func->wield(this_object())))
    {
        // Custom wield function handled the wield
    }
    else
    {
        // Default wielding messages
        if (wielded_in_hand == W_BOTH)
        {
            write("You wield " + LANG_THESHORT(this_object()) + " in both your hands.\n");
        }
        else if (wielded_in_hand < W_FOOTR)
        {
            write("You wield " + LANG_THESHORT(this_object()) + " in your " +
                (wielded_in_hand == W_RIGHT ? "right" : "left") + " hand.\n");
        }
        else
        {
            write("You wield " + LANG_THESHORT(this_object()) + " on your " +
                (wielded_in_hand == W_FOOTR ? "right" : "left") + " foot.\n");
        }

        say(QCTNAME(this_player()) + " wields " +
            ({string}) this_player()->query_possessive() + " " +
            QSHORT(this_object()) + ".\n");
    }

    // If wield succeeded or was handled by wield function
    if (intp(wret) && wret >= 0)
    {
        wielded = 1;
        set_adj("wielded");
        remove_adj("unwielded");
        return 1;
    }

    // Something went wrong during wielding - attempt to unwield
    wielder->unwield(this_object());
    
    // Return appropriate error message
    return stringp(wret) ? wret : 
        "You cannot wield " + LANG_THESHORT(this_object()) + ".\n";
}


/*
 * Function name: command_wield
 * Description  : Called to wear this weapon.
 * Returns      : See wield_me()
 */
mixed command_wield()
{
    return wield_me();
}


/*
 * Function name: unwield_me
 * Description  : When the player tries to unwield this weapon, this function
 *                is called. If the player managed to unwield the weapon,
 *                the message is printed in this routine. Error messages are
 *                returned.
 * Returns      : int 1  - success
 *                int 0  - failure: the weapon was not wielded to begin with.
 *                string - a fail message (nothing is printed).
 */
public nomask mixed unwield_me()
{
    mixed wret;

    if (!wielded || !wielder)
    {
        return 0;
    }

    wret = 0;

    if (!wield_func || !(wret = ({mixed}) wield_func->unwield(this_object())))
    {
        if (check_seen(wielder))
        {
            wielder->catch_tell("You stop wielding " + LANG_THESHORT(this_object()) + ".\n");
        }
        else
        {
            wielder->catch_tell("You stop wielding something.\n");
        }

        tell_room(environment(wielder), QCTNAME(wielder) +
            " stops wielding " + ({string}) wielder->query_possessive() + " " +
            QSHORT(this_object()) + ".\n", ({ wielder }));
    }

    if (intp(wret) && (wret >= 0))
    {
        wielder->unwield(this_object());
        wielded = 0;
        remove_adj("wielded");
        add_adj("unwielded");
        wielded_in_hand = wep_hands;
        return 1;
    }

    return (stringp(wret) ? wret : "");
}


/*
 * Function name: command_unwield
 * Description  : Called to wear this weapon.
 * Returns      : See unwield_me()
 */
mixed command_unwield()
{
    return unwield_me();
}


/*
 * Function name: leave_env
 * Description  : When the weapon leaves a certain inventory this function
 *                is called. If the weapon is wielded, we unwield it. If you
 *                mask this function you _must_ make a call to ::leave_env
 * Arguments    : object from - the object the weapon leaves.
 *                object desc - the destination of the weapon.
 */
void leave_env(object from, object dest)
{
    ::leave_env(from, dest);

    /* No need to continue if the weapon isn't wielded */
    if (!wielded)
        return;

    /* Notify the wielder that they've stopped wielding the weapon */
    if ((!wield_func || !({mixed}) wield_func->unwield(this_object())) && wielder)
    {
        tell_object(wielder, "You stop wielding " + LANG_THESHORT(this_object()) + ".\n");
    }

    /* Update wielder state and weapon properties */
    wielder->unwield(this_object());
    remove_adj("wielded");
    add_adj("unwielded");
    wielded = 0;
}


/*
 * Function name: set_hit
 * Description  : Set the to hit value in the weapon. This relates to how easy
 *                it is to make a hit with this weapon.
 *                When the lock has been set, no changes are allowed anymore.
 * Arguments    : int class - the hit value.
 */
void set_hit(int class)
{
    if (query_lock())
        return;

    wep_hit = class;
}


/*
 * Function name: query_hit
 * Description  : Query the to hit value in the weapon.
 * Returns      : int - the hit value.
 */
int query_hit()
{
    return wep_hit;
}


/*
 * Function name: set_pen
 * Description  : Set penetration value of the weapon. This relates to how
 *                badly it hurts when you are hit by this weapon.
 *                When the lock has been set, no changes are allowed anymore.
 * Arguments    : int class - the pen value.
 */
void set_pen(int class)
{
    if (query_lock())
        return;

    wep_pen = class;
}


/*
 * Function name: query_pen
 * Description  : Query penetration value of the weapon.
 * Returns      : int - the pen value.
 */
int query_pen()
{
    return wep_pen;
}


/*
 * Function name: set_pm
 * Description  : Set the modifiers for damage types. These modify the pen
 *                value for the individual damage types.
 *                When the lock has been set, no changes are allowed anymore.
 * Arguments    : int *list - array of modifiers ({ impale, slash, bludgeon })
 *                The sum of the modifiers should be 0 and a modifier for
 *                a damage type that is not used should also be 0.
 */
void set_pm(int *list)
{
    if (query_lock())
        return;

    if (F_LEGAL_AM(list))
        m_pen = list;
}


/*
 * Function name: query_pm
 * Description  : Query the modifiers for the damage types.
 * Returns      : int * - array of modifiers ({ impale, slash, bludgeon })
 */
int *query_pm()
{
    return m_pen + ({});
}


/*
 * Function name: query_modified_pen
 * Description  : Query the effective (modified) pen values for the different
 *                damage types. These take corrosion and dulling into account.
 * Returns      : int * - array of modified pen values ({ impale, slash, blg })
 */
int *query_modified_pen()
{
    int *m_pent, i, du, co, pen;

    du = dull - repair_dull;
    co = corroded - repair_corr;
    pen = ({int}) this_object()->query_pen();

    m_pent = allocate(W_NO_DT);

    for (i = 0; i < W_NO_DT; i++)
    {
        if (!pointerp(m_pen))
            m_pent[i] = pen;
        else if (i >= sizeof(m_pen))
            m_pent[i] = pen + (i ? m_pen[0] : 0);
        else
            m_pent[i] = pen + m_pen[i];
    }

    return ({ m_pent[0] - 2 * (du + co) / 3, m_pent[1] - du - co,
                m_pent[2] - (du + co) / 3 });
}


/*
 * Function name: set_wt
 * Description  : Set the weapon type, W_SWORD, W_AXE, ...
 *                When the lock has been set, no changes are allowed anymore.
 * Arguments    : int type - the weapon type.
 */
void set_wt(int type)
{
    int *likely;

    if (query_lock())
        return;

    if (type >= 0 && type < W_NO_T)
    {
        wep_wt = type;
        likely = W_DRAWBACKS[type];
        likely_dull = likely[0];
        likely_corr = likely[1];
        likely_break = likely[2];
    }
}


/*
 * Function name: query_wt
 * Description  : Query the type of weapon.
 * Returns      : int - the weapon type.
 */
int query_wt()
{
    return wep_wt; 
}


/*
 * Function name: set_dt
 * Description  : Set the damage type of the weapon. This should naturally
 *                make a logical match with the weapon type. The damage type
 *                is a binary combination of W_IMPALE | W_SLASH | W_BLUDGEON
 *                  - impale   = for stabbing
 *                  - slash    = for cutting
 *                  - bludgeon = for (dull) hitting with force
 *                When the lock has been set, no changes are allowed anymore.
 * Arguments    : int type - the damage type.
 */
void set_dt(int type)
{
    if (query_lock())
        return;

    if (F_LEGAL_DT(type))
        wep_dt = type;
}


/*
 * Function name: query_dt
 * Description  : Query the damage type of the weapon.
 * Returns      : int - the damage type of the weapon.
 */
int query_dt()
{
    return wep_dt; 
}


/*
 * Function name: set_hands
 * Description  : Set how the weapon should be wielded, W_ANYH, W_BOTH, ...
 *                When the lock has been set, no changes are allowed anymore.
 * Arguments    : int which - the hands.
 */
void set_hands(int which)
{
    if (query_lock())
        return;

    if (F_LEGAL_HANDS(which))
        wep_hands = which;
}


/*
 * Function name: query_hands
 * Description  : Find out the hands that we can use for this weapon.
 * Returns      : int - the hands.
 */
int query_hands()
{
    return wep_hands; 
}


/*
 * Function name: query_attack_id
 * Description  : When wielded, this returns in which hands the weapon is
 *                actually wielded.
 * Returns      : int - the hands, or 0 if not wielded.
 */
int query_attack_id()
{
    return wielded_in_hand; 
}


/*
 * Function name: query_slots
 * Description  : Find out the tool slot(s) that the weapon occupies now.
 * Returns      : int * - the tool slots.
 */
int *query_slots()
{
    int abit, *hids;

    for (hids = ({}), abit = 2; abit <= wielded_in_hand; abit *= 2)
    {
        if (wielded_in_hand & abit)
        {
            hids = hids + ({ abit });
        }
    }
    return hids;
}


/*
 * Function name: query_protects
 * Description  : Find out the slots the weapon protects. A weapon protects
 *                the whole arm when wielded in a hand.
 * Returns      : int * - the slots.
 */
int *query_protects()
{
    switch (wielded_in_hand)
    {
        case W_LEFT:
            return ({ A_L_ARM });
        case W_RIGHT:
            return ({ A_R_ARM });
        case W_BOTH:
            return ({ A_L_ARM, A_R_ARM });
        case W_FOOTR:
            return ({ A_R_FOOT });
        case W_FOOTL:
            return ({ A_L_FOOT });
        default:
            return ({ });
    }
}


/*
 * Function name: set_wf
 * Description  : To do special checks or processing when a weapon is wielded
 *                or unwielded. the routines wield and unwield can be defined
 *                either in this object, or in an external object.
 *
 *                mixed wield(object weapon) { }
 *                mixed unwield(object weapon) { }
 *
 *                Note that while wield() may operate on this_player(), the
 *                unwield() routine cannot rely on that. In unwield(), use
 *                the query_wielded() routine to find out who is wielding it.
 *
 *                Those functions can return:
 *                   0 - No effect; the weapon can be wielded / unwielded.
 *                   1 - It can be wielded / unwielded but no text should be
 *                       printed (it was done in the function).
 *                  -1 - It can not be wielded / unwielded. A default fail
 *                       message will be written.
 *                 str - It can not be wielded / unwielded. The string 'str'
 *                       is the fail message to print.
 *
 * Arguments     : object obj - the object that defines the routines.
 */
void set_wf(object obj)
{
    if (query_lock())
        return;

    wield_func = obj;
}


#if 0
/*
 * Function name: wield
 * Description  : This function might be called when someone tries to wield
 *                this weapon. To have this function called, use the function
 *                set_wf().
 *                Note: this routine does not actually exist in /std/weapon.
 *                      A trick is used to fool the document maker.
 * Arguments    : object obj - the weapon someone tried to wield.
 * Returns      : int  0 - wield this weapon normally.
 *                     1 - wield the weapon, but print no messages.
 *                    -1 - do not wield the weapon, use default messages.
 *                string - do not wield the weapon, use this fail message.
 */
public mixed wield(object obj)
{
    return 0;
}


/*
 * Function name: unwield
 * Description  : This function might be called when someone tries to unwield
 *                this weapon. To have this function called, use the function
 *                set_wf().
 *                Note: this routine does not actually exist in /std/weapon.
 *                      A trick is used to fool the document maker.
 * Arguments    : object obj - the weapon to stop wielding.
 * Returns      : int  0 - the weapon can be unwielded normally.
 *                     1 - unwield the weapon, but print no messages.
 *                    -1 - do not unwield the weapon, print default messages.
 *                string - do not unwield the weapon, use this fail message.
 */
public mixed unwield(object obj)
{
    return 0;
}
#endif


/*
 * Function name: set_corroded
 * Description  : Use this to increases the corroded status on weapons.
 * Arguments    : int cond - The new condition we want (can only be raised)
 * Returns      : int - 1 if new condition accepted, 0 if no corrosion.
 */
int set_corroded(int corr)
{
    if (corr > corroded)
    {
        corroded = corr;
        if (F_WEAPON_BREAK(dull - repair_dull, corroded - repair_corr, likely_break))
            call_out(#'remove_broken, 0, 0);

        if (wielded && wielder)
            wielder->update_weapon(this_object());

        return 1;
    }

    return 0;
}


/*
 * Function name: query_corroded
 * Description  : Returns how many times this weapon has become corroded.
 * Returns      : int - The number of times.
 */
int query_corroded()
{
    return corroded;
}


/*
 * Function name: set_likely_corr
 * Description:   Set how likely it is this weapon will corrode when in acid
 *                or something like that. 0 means it won't corrode at all.
 * Arguments:     i - how likely it will corrode, probably corrode if random(i)
 *                    [0, 20] recommended
 */
void set_likely_corr(int i)
{
    likely_corr = i; 
}


/*
 * Function name: query_likely_corr
 * Description:   Query how likely the weapon will corrode, use it to test
 *                if weapon survived your rustmonster or acid pool :)
 */
int query_likely_corr()
{ 
    return likely_corr; 
}


/*
 * Function name: set_dull
 * Description  : Use this to increases the dull status on weapons.
 * Arguments    : int cond - The new condition we want (can only be raised)
 * Returns      : int - 1 if new condition accepted, 0 if not.
 */
int set_dull(int du)
{
    if (du > dull)
    {
        dull = du;

        if (F_WEAPON_BREAK(dull - repair_dull, corroded - repair_corr, likely_break))
            call_out(#'remove_broken, 0, 0);

        if (wielded && wielder)
            wielder->update_weapon(this_object());

        return 1;
    }

    return 0;
}


/*
 * Function name: query_dull
 * Description  : Returns how many times this weapon has become duller.
 * Returns      : int - The number of times.
 */
int query_dull() 
{ 
    return dull; 
}


/*
 * Function name: set_likely_dull
 * Description:   Set how likely the weapon will get dull or in case of a club
 *                or mace, wear down in combat Mainly used from did_hit().
 * Arguments:     i - how likely [0, 30] recommended
 */
void set_likely_dull(int i) 
{ 
    likely_dull = i; 
}


/*
 * Function name: query_likely_dull
 * Description:   How likely it is this weapon will become duller when used
 * Returns:       int - How likely it is
 */
int query_likely_dull() 
{ 
    return likely_dull; 
}


/*
 * Function name: set_likely_break
 * Description:   Set how likely the weapon is to break if you use it.
 * Argument:      i - How likely, [0, 20] recommended
 */
void set_likely_break(int i) 
{ 
    likely_break = i; 
}


/*
 * Function name: query_likely_break
 * Description:   How likely is it this weapon will break with use
 * Returns:       int - How likely it is
 */
int query_likely_break() 
{ 
    return likely_break; 
}


/*
 * Function name: remove_broken
 * Description  : The weapon got broken so the player has to stop
 *                wielding it.
 * Arguments    : int silent - true if no message is to be genereated
 *                             about the fact that the weapon broke.
 */
varargs void remove_broken(int silent = 0)
{
    /* If the weapon is not wielded, we only adjust the broken information
     * by adding the adjective and the property. We do this within the
     * if-statement since we do not want to screw the message that may
     * be displayed later. When the property is added, the adjective is
     * added automatically.
     */
    if (!wielded || !wielder)
    {
        add_prop(OBJ_I_BROKEN, 1);
        return;
    }

    if (objectp(wield_func))
    {
        wield_func->unwield(this_object());
    }

    /* If the wizard so chooses, these messages may be suppressed. */
    if (!silent)
    {
        tell_object(wielder, "The " + short(wielder) + " breaks!!!\n");
        tell_room(environment(wielder), "The " + QSHORT(this_object()) +
            " wielded by " + QTNAME(wielder) + " breaks!!!\n", ({ wielder }));
    }

    /* Force the player to unwield the weapon and then adjust the broken
     * information by adding the property and the adjective.
     */
    wielder->unwield(this_object());
    remove_adj("wielded");
    add_adj("unwielded");
    add_prop(OBJ_I_BROKEN, 1);
    wielded = 0;
}


/*
 * Function name: set_repair_dull
 * Description:   When trying to repair the weapon, call this function. Repairs
 *                can only increase the repair factor. (This is sharpening)
 * Arguments:     int rep - The new repair number
 * Returns:       int - 1 if new repair status accepted
 */
int set_repair_dull(int rep)
{
    if (rep > repair_dull && F_LEGAL_WEAPON_REPAIR_DULL(rep, dull))
    {
        repair_dull = rep;

        if (wielded && wielder)
            wielder->update_weapon(this_object());

        return 1;
    }

    return 0;
}


/*
 * Function name: query_repair_dull
 * Description:   How many times has this weapon been sharpened
 * Returns:       int - The number of times
 */
int query_repair_dull() 
{ 
    return repair_dull; 
}


/*
 * Function name: set_repair_corr
 * Description:   When trying to repair the weapon, call this function. Repairs
 *                can only increase the repair factor. This repairs corroded.
 * Arguments:     int rep - The new repair number
 * Returns:       int 1 - if new repair status accepted
 */
int set_repair_corr(int rep)
{
    if (rep > repair_corr && F_LEGAL_WEAPON_REPAIR_CORR(rep, corroded))
    {
        repair_corr = rep;

        if (wielded && wielder)
            wielder->update_weapon(this_object());

        return 1;
    }

    return 0;
}


/*
 * Function name: query_repair_corr
 * Description:   How many times this weapon has been repaired from corrosion
 * Returns:       int - How many times
 */
int query_repair_corr() 
{ 
    return repair_corr; 
}


/*
 * Function name: set_weapon_hits
 * Description:   By setting the hits counter you will have influence over how
 *                likely the weapon is to get in a worse condition. The hits
 *                variable keeps track of how many times this piece of weapon
 *                has hit something.
 * Argument:      int new_hits
 */
public void set_weapon_hits(int new_hits) 
{ 
    hits = new_hits; 
}


/*
 * Function name: query_weapon_hits
 * Description:   hits variable keeps track of how many times this weapon has
 *                hit something.
 * Returns:       The number of times
 */
public int query_weapon_hits() 
{ 
    return hits; 
}


/*
 * Function name: add_prop_obj_i_value
 * Description:   Someone is adding the value prop to this object.
 * Arguments:     mixed val - The new value
 * Returns:       int - 1 if not to let the val variable through to the prop
 */
int add_prop_obj_i_value(mixed val)
{
    if (!max_value)
    {
        max_value = 10;
        return 0;
    }

    if (intp(val) && val)
        max_value = val;

    return 1;
}


/*
 * Function name: query_value
 * Description  : What's the value of this armour based on corrosion status.
 * Returns      : int - the value.
 */
int query_value()
{
    if (query_prop(OBJ_I_BROKEN))
        return 0;

    return max_value * F_WEAPON_VALUE_REDUCE(dull - repair_dull, corroded - repair_corr) / 100;
}


/*
 * Function Name: query_repair_cost_dull
 * Description  : Returns the cost to repair this weapon one dull step.
 * Returns      : int - the cost in cc
 */
int query_repair_cost_dull()
{
    return max(max_value, F_VALUE_WEAPON(query_hit(), query_pen())) * F_WEAPON_REPAIR_COST_FACTOR / 100;
}


/*
 * Function Name: query_repair_cost_corr
 * Description  : Returns the cost to repair this weapon from one level of
 *                corrosion
 * Returns      : int - the cost in cc
 */
int query_repair_cost_corr()
{
    return max(max_value, F_VALUE_WEAPON(query_hit(), query_pen())) * F_WEAPON_REPAIR_COST_FACTOR / 100;
}


/*
 * Function name: query_wf
 * Description  : Query if/what object defines wield/unwield functions.
 *                See set_wf() for details.
 * Returns      : object
 */
object query_wf() 
{ 
    return wield_func; 
}


/*
 * Function name: try_hit
 * Description  : Called from living when weapon used. This routine can be
 *                redefined to skip the use of this weapon (for this target).
 * Arguments    : object target - Who I intend to hit.
 * Returns      : int - 0 if weapon miss. If true it might hit.
 */
int try_hit(object target)
{ 
    return 1; 
}


/*
 * Function name: did_hit
 * Description:   Tells us that we hit something. Should produce combat
 *                messages to all relevant parties. If the weapon
 *                chooses not to handle combat messages then a default
 *                message is generated.
 * Arguments:     aid:   The attack id
 *                hdesc: The hitlocation description.
 *                phurt: The %hurt made on the enemy
 *                enemy: The enemy who got hit
 *                dt:    The current damagetype
 *                phit:  The %success that we made with our weapon
 *                dam:   The actual damage caused by this weapon in hit points
 *                hid:   The hitlocation id
 * Returns:       True if it handled combat messages, returning a 0 will let
 *                the normal routines take over
 */
public varargs int did_hit(
    int aid,
    string hdesc,
    int phurt,
    object enemy,
    int dt,
    int phit,
    int dam,
    int hid)
{
    hits++;

    if (F_WEAPON_CONDITION_DULL(hits, wep_pen, likely_dull))
    {
        hits = 0;
        set_dull(query_dull() + 1);
    }

    return 0;
}


/*
 * Function name: check_weapon
 * Description  : Check file for security.
 * Returns      : int 1 - always.
 */
nomask int check_weapon()
{
    return 1;
}


/*
 * Function name: set_default_weapon
 * Description  : Configures the weapon by replacing up to six calls with
 *                just one. For details, see the respective functions.
 * Arguments    : int hit    - set_hit(hit)
 *                int pen    - set_pen(pen)
 *                int wt     - set_wt(wt)
 *                int dt     - set_dt(dt)
 *                int hands  - set_hands(hands)
 *                object obj - set_wf(obj)
 */
varargs void set_default_weapon(
    int hit,
    int pen,
    int wt,
    int dt,
    int hands,
    object obj)
{
    /* Sets the weapon "to hit" value. */
    set_hit(hit ? hit : 5);

    /* Sets the weapon penetration value. */
    set_pen(pen ? pen : 10);

    /* Set the weapon type. */
    set_wt(wt ? wt : W_FIRST);

    /* Set the damage type. */
    set_dt(dt ? dt : (W_IMPALE | W_SLASH));

    /* Set the hand(s) used to wield the weapon. */
    set_hands(hands ? hands : W_NONE);

    /* Sets the name of the object that contains the function to call for
     * extra defined wield() and unwield() functions. */
    if (obj) set_wf(obj);
}


/*
 * Function name: query_wield_desc
 * Description  : Describe this weapon as wielded by a something.
 * Argumensts   : string p: Possessive description of wielder
 * Returns      : string - the description.
 */
public nomask string query_wield_desc(string p)
{
    string str;

    /* Allow masking of the short. */
    if (check_seen(this_player()))
        str = LANG_ADDART(({string}) this_object()->short(this_player()));
    else
        str = "something";

    switch(wielded_in_hand)
    {
    case W_RIGHT:return str + " in " + p + " right hand";
    case W_LEFT: return str + " in " + p + " left hand";
    case W_BOTH: return str + " in both hands";
    case W_FOOTR:return str + " on " + p + " right foot";
    case W_FOOTL:return str + " on " + p + " left foot";
    }
    return str;
}


/*
 * Function name: update_prop_settings
 * Description:   Will uppdate weight and value of this object to be legal
 */
nomask void update_prop_settings()
{
    if (query_prop(OBJ_I_VALUE) < F_VALUE_WEAPON(wep_hit, wep_pen) &&
            !query_prop(OBJ_I_IS_MAGIC_WEAPON))
        add_prop(OBJ_I_VALUE, F_VALUE_WEAPON(wep_hit, wep_pen));

    if (F_WEIGHT_FAULT_WEAPON(query_prop(OBJ_I_WEIGHT), wep_pen, wep_wt) &&
            !query_prop(OBJ_I_IS_MAGIC_WEAPON))
        add_prop(OBJ_I_WEIGHT, F_WEIGHT_DEFAULT_WEAPON(wep_pen, wep_wt));
}


/*
 * Function name: query_wielded
 * Description  : Find out whether this weapon is wielded or not.
 * Returns      : object - the wielder, or 0 if it isn't wielded.
 */
object query_wielded()
{
    if (wielded)
        return wielder;

    return 0;
}


/*
 * Function name: query_am
 * Description  : Called when wielding the weapon, to check for the parry.
 * Returns      : int * - the armour modifier.
 */
public nomask int *query_am()
{
    return ({ -3, 2, 1 });
}


/*
 * Function name: query_ac
 * Description  : Called when wielding the weapon, to check for parry.
 * Returns      : int - the AC value for the weapon.
 */
public nomask int query_ac()
{
    int ac = (wep_hit / 2) + ac_modifier;

    if (wielder)
    {
        ac += ((({int}) wielder->query_skill(SS_PARRY) * wep_hit) / 500);
    }

    return ac;
}


/*
 * Function name: set_ac_modifier
 * Description  : A modifier on the armour class for the weapon. A weapon by
 *                default has an AC related to the weapon to-hit value. When
 *                setting this value, you must document in the code why you
 *                change it, especially if you set it to a positive value.
 *                The modifier is added to the calculated AC value.
 * Arguments    : int ac_mod - the modifier, in range -10 .. 5
 */
public nomask void set_ac_modifier(int ac_mod)
{
    if ((ac_mod <= 5) || (ac_mod >= -10))
    {
        ac_modifier = ac_mod;
    }
}


/*
 * Function name: query_ac_modifier
 * Description  : Returns the AC modifier for the weapon. See set-routine.
 * Returns      : int - the modifier for the AC value of the weapon.
 */
public nomask int query_ac_modifier()
{
    return ac_modifier;
}


/*
 * Function name: stat_object
 * Description  : This function is called when a wizard wants to get more
 *                information about an object.
 * Returns      : string - the info.
 */
string stat_object()
{
    string str;

    str = ::stat_object();

    str += "Hit: " + wep_hit + "\t\tPen: " + wep_pen + "\n";

    str += "Type: " + wep_wt + "\n";
    str += "Hands: " + wep_hands + "\n";

    return str;
}


/*
 * Function name: wep_condition_desc
 * Description  : Returns the description of the condition of the weapon.
 * Returns      : string - the descriptive text.
 */
string wep_condition_desc()
{
    string rust_desc = "";
    string wear_desc = "";
    
    if (query_prop(OBJ_I_BROKEN))
    {
        return "It is broken and unusable.\n";
    }

    // Determine the rust/corrosion description
    int corr_level = corroded - repair_corr;

    switch (corr_level)
    {
        case 0:
            rust_desc = ""; 
            break;
        case 1:
            rust_desc = "You notice some small rust marks on it.\n"; 
            break;
        case 2:
            rust_desc = "You can spot patches of rust spreading across the surface.\n"; 
            break;
        case 3:
            rust_desc = "The weapon is significantly corroded with rust covering much of its surface!\n"; 
            break;
        case 4:
        case 5:
            rust_desc = "The weapon appears heavily corroded, as if it has been exposed to acid!\n"; 
            break;
        default:
            rust_desc = "The weapon is so severely corroded it looks like it could fall apart at any moment.\n"; 
            break;
    }

    // Determine the wear/dullness description
    int dull_level = dull - repair_dull;

    switch (dull_level)
    {
        case 0:
            wear_desc = "in pristine condition"; 
            break;
        case 1:
            wear_desc = "in excellent condition"; 
            break;
        case 2:
            wear_desc = "showing minor signs of use"; 
            break;
        case 3:
            wear_desc = "bearing noticeable battle scars"; 
            break;
        case 4:
        case 5:
            wear_desc = "showing significant wear from combat"; 
            break;
        case 6:
        case 7:
        case 8:
            wear_desc = "badly damaged and in urgent need of repair"; 
            break;
        default:
            wear_desc = "so worn and damaged it could break during combat"; 
            break;
    }

    return rust_desc + "It appears to be " + wear_desc + ".\n";
}


/*
 * Function name: weapon_type
 * Description  : This function shuold return the type of the weapon in text.
 * Returns      : string - the name of the weapon type.
 */
string weapon_type()
{
    if (wep_wt >= W_NO_T)
    {
        return "weapon";
    }

    return W_NAMES[wep_wt];
}


/*
 * Function name: wep_usage_desc
 * Description  : This function returns the usage of this weapon. It is
 *                usually printed from the appraise function. The string
 *                includes the type of the weapon and the location where it
 *                should be wielded.
 * Returns      : string - the description.
 */
string wep_usage_desc()
{
    string hand;

    switch (wep_hands)
    {
        case W_RIGHT:
            hand = "in the right hand";
            break;
        case W_LEFT:
            hand = "in the left hand";
            break;
        case W_BOTH:
            hand = "in both hands";
            break;
        case W_FOOTR:
            hand = "on the right foot";
            break;
        case W_FOOTL:
            hand = "on the left foot";
            break;
        case W_ANYH:
            hand = "in any hand";
            break;
        default:
            hand = "by some strange creature";
            break;
    }

    return ("The " + weapon_type() + " is made to be wielded " + hand + ".\n");
}


/*
 * Function name: appraise_object
 * Description  : Someone tries to appraise the object. We add information
 *                about the way you should use this weapon.
 * Arguments    : int num - a number based on the skill of the person.
 */
void appraise_object(int num)
{
    ::appraise_object(num);

    write(wep_usage_desc());
}


/*
 * Function name: did_parry
 * Description:   Tells us that this weapon was used to parry an attack. It can
 *                be used to wear down a weapon. Note that his method is called
 *                before the combat messages are printed out.
 * Arguments:     object att - the attacker
 *                int aid:   - the attack id
 *                int dt     - the damagetype
 */
public varargs void did_parry(object att, int aid, int dt)
{
}


/*
 * Function name: may_not_recover
 * Description  : This function will be true if the weapon may not recover.
 * Returns      : int - 1 - no recovery, 0 - recovery.
 */
nomask int may_not_recover()
{
    return will_not_recover;
}


/*
 * Function name: may_recover
 * Description  : In some situations it is undesirable to have a weapon
 *                not recover. This function may then be used to force the
 *                weapon to be recoverable.
 *
 *                This function may _only_ be called when a craftsman sells
 *                a weapon he created to a player! It may expressly not be
 *                called in weapons that are to be looted from NPC's!
 */
nomask void may_recover()
{
    will_not_recover = 0;
}


/*
 * Function name: set_may_not_recover
 * Description  : Call this to force the weapon to be non-recoverable.
 */
nomask void set_may_not_recover()
{
    will_not_recover = 1;
}


/*
 * Function name: query_wep_recover
 * Description  : Return the recover strings for changing weapon variables.
 * Returns      : string - a recover string.
 */
string query_wep_recover()
{
    return ("#WEP#" + hits + "#" + dull + "#" + corroded + "#" +
        repair_dull + "#" + repair_corr + "#" + query_prop(OBJ_I_BROKEN) +
        "#");
}


/*
 * Function name: init_wep_recover
 * Description  : Initialize the weapon variables at recover.
 * Arguments    : string arg - the variables to recover
 */
void init_wep_recover(string arg)
{
    string foobar;
    int    broken;

    sscanf(arg, "%s#WEP#%d#%d#%d#%d#%d#%d#%s", foobar,
        hits, dull, corroded, repair_dull, repair_corr, broken, foobar);

    if (broken != 0)
    {
        add_prop(OBJ_I_BROKEN, broken);
    }
}


/*
 * Function name: query_recover
 * Description  : Called to check whether this weapon is recoverable.
 *                If you have variables you want to recover yourself,
 *                you have to redefine this function, keeping at least
 *                the filename and the weapon recovery variables.
 *                If, for some reason, you do not want your weapon to
 *                recover, you should define the function and return 0.
 * Returns      : string - the recovery string, or 0 if not recoverable.
 */
public string query_recover()
{
    // If this weapon is marked as not recoverable, return 0
    if (will_not_recover)
        return 0;
    
    return MASTER + ":" + query_wep_recover();
}


/*
 * Function name: init_recover
 * Description  : When the object recovers, this function is called
 *                to set the necessary variables. If you redefine the
 *                function, you must add a call to init_wep_recover
 *                with the string that you got after querying
 *                query_wep_recover.
 * Arguments    : string argument - the arguments to parse
 */
public void init_recover(string arg)
{
    if (!stringp(arg) || arg == "")
    {
        return;
    }
    
    init_wep_recover(arg);
}
