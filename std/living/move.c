/*
 * /std/living/move.c
 *
 * This is a subpart of living.c
 *
 * All movement related routines are coded here.
 */

#include <filter_funs.h>
#include <macros.h>
#include <options.h>
#include <std.h>
#include <stdproperties.h>

/*
 * Global static variable. Used here because a global variable is at present
 * the most efficient way to share a mapping between objects without making
 * copies.
 * WARNING: Only query this mapping as move_opposites[x]; do not assign!
 */
static private mapping move_opposites = ({mapping}) SECURITY->query_move_opposites();


/*
 * Function name: move_reset
 * Description  : Reset the move module of the living object.
 */
static nomask void move_reset()
{
    set_m_in(LD_ALIVE_MSGIN);
    set_m_out(LD_ALIVE_MSGOUT);

    set_mm_in(LD_ALIVE_TELEIN);
    set_mm_out(LD_ALIVE_TELEOUT);
}


/*
 * Function name: move_living
 * Description:   Posts a move command for a living object somewhere. If you
 *                have a special exit like 'climb tree' you might want to
 *                use set_dircmd() and set it to 'tree' in the room to allow
 *                teammembers to follow their leader.
 * Arguments:     how:      The direction of travel, like "north".
 *                          "X" for teleportation, team does not follow.
 *                          "M" if you write leave and arrive messages yourself.
 *                to_dest:  Destination
 *                dont_follow: A flag to indicate group shall not follow this
 *                          move if this_object() is leader
 *                no_glance: Don't look after move.
 *
 * Returns:       Result code of move:
 *                      0: Success.
 *
 *                      3: Can't take it out of it's container.
 *                      4: The object can't be inserted into bags etc.
 *                      5: The destination doesn't allow insertions of objects.
 *                      7: Other (Error message printed inside move() func)
 */
public varargs int move_living(string how, mixed to_dest, int dont_follow, int no_glance)
{
    int    index, size, invis;
    object *team, *dragged, env, oldtp;
    string verb = query_verb();
    string com, msgout, msgin;
    mixed msg;
    string from_desc;

    log_debug(" === MOVE LIVING Verb: %s ===", to_string(verb));
    log_debug("Moved object: %s", to_string(this_object()));
    log_debug("Move to: %s", to_string(to_dest));
    log_debug("Move how: %s", to_string(how));
    log_debug("Move dont_follow: %d", dont_follow);
    log_debug("Move no_glance: %d", no_glance);

    oldtp = this_player();

    if (!objectp(to_dest))
    {
        log_debug("Move to_dest is not an object, calling find_object");
        msg = LOAD_ERR(to_dest);
        to_dest = find_object(to_dest);
    }

    if (stringp(msg))
    {
        log_debug("Move msg is a string, calling find_object");
        if (!environment(this_object()))
        {
            log_debug("Move error: %s", msg);
            tell_object(this_object(), "PANIC Move error: " + msg);
            to_dest = ({string}) this_object()->query_default_start_location();
            msg = LOAD_ERR(to_dest);
            to_dest = find_object(to_dest);
        }
        else
        {
            log_debug("Move error: %s", msg);
            tell_object(this_object(), msg);
            SECURITY->log_loaderr(to_dest, environment(this_object()), how,
                previous_object(), msg);
            return 7;
        }
    }

    if (!({int}) to_dest->query_prop(ROOM_I_IS))
    {
        log_debug("Move error: %s", msg);
        return 7;
    }

    if (!how)
    {
        log_debug("How missing");
        return move(to_dest, 1);
    }

    if (how == "M")
    {
        log_debug("Move M");
        msgin = 0;
        msgout = 0;
    }
    else if (how == "X")
    {
        log_debug("Move X");
        msgin = ({string}) this_object()->query_mm_in() + "\n";
        msgout = ({string}) this_object()->query_mm_out() + "\n";
        /* When transing, the team does not follow. */
        dont_follow = 1;
    }
    else
    {
        log_debug("Move else, direction: %s", how);
        if (query_prop(LIVE_I_SNEAK))
        {
            log_debug("Move sneaking");
            msgin = ({string}) this_object()->query_m_in() + " sneaking";
            msgout = ({string}) this_object()->query_m_out() + " sneaking " + how + ".\n";
        }
        else
        {
            log_debug("Move not sneaking");
            msgin = ({string}) this_object()->query_m_in();
            msgout = ({string}) this_object()->query_m_out() + " " + how + ".\n";
        }

        if (sizeof(from_desc =
            ({string}) environment()->query_prop(ROOM_S_EXIT_FROM_DESC)))
        {
            log_debug("Move from_desc: %s", from_desc);
            msgin += " " + from_desc;
        }
        else if (sizeof(from_desc = move_opposites[verb]))
        {
            log_debug("Move opposite: %s", from_desc);
            msgin += " from " + from_desc + ".";
        }
        else
        {
            log_debug("Move no opposite");
            msgin += ".";
        }

        msgin += "\n";
    }

    invis = query_prop(OBJ_I_INVIS);

    /* Make us this_player() if we aren't already. */
    if (this_object() != this_player())
    {
        log_debug("Move this_player() != this_object()");
        set_this_player(this_object());
    }

    if (env = environment(this_object()))
    {
        log_debug("Move env: %s", to_string(env));
        /* Update the last room settings. */
        add_prop(LIVE_O_LAST_ROOM, env);
        add_prop(LIVE_S_LAST_MOVE, verb);

        /* Update the hunting status */
        this_object()->adjust_combat_on_move(1);

        /* Leave footprints. */
        if (!({int}) env->query_prop(ROOM_I_INSIDE) &&
            (({int}) env->query_prop(ROOM_I_TYPE) == ROOM_NORMAL) &&
            !({int}) query_prop(LIVE_I_NO_FOOTPRINTS))
        {
            log_debug("Move leaving footprints");
            env->add_prop(ROOM_S_DIR, ({ how, query_race_name() }) );
        }

        /* Report the departure. */
        if (msgout)
        {
            log_debug("Move msgout: %s", msgout);
            if (invis)
            {
                log_debug("Move invis");
                say( ({ "(" + METNAME + ") " + msgout,
                    TART_NONMETNAME + " " + msgout,
                    "" }) );
            }
            else
            {
                log_debug("Move not invis");
                say( ({ METNAME + " " + msgout,
                    TART_NONMETNAME + " " + msgout,
                    "" }) );
            }
        }
    }

    if (!query_prop(LIVE_I_SNEAK))
    {
        log_debug("Move from hidden not sneaking");
        remove_prop(OBJ_I_HIDE);
    }
    else
    {
        log_debug("Move from hidden sneaking");
        remove_prop(LIVE_I_SNEAK);
    }

    if (index = move(to_dest))
    {
        log_debug("Move error: %d", index);
        return index;
    }

    if (msgin)
    {
        log_debug("Move msgin: %s", msgin);
        if (invis)
        {
            log_debug("Move invis");
            say( ({ "(" + METNAME + ") " + msgin,
                ART_NONMETNAME + " " + msgin,
                "" }) );
        }
        else
        {
            log_debug("Move not invis");
            say( ({ METNAME + " " + msgin,
                ART_NONMETNAME + " " + msgin,
                "" }) );
        }
    }

    /* Take a look at the room you've entered, before the combat adjust.
     * Only interactive players bother to look. Don't waste our precious
     * CPU-time on NPC's.
     */
    if (interactive(this_object()) &&
        !no_glance)
    {
        log_debug("Move no_glance: %d", no_glance);
        this_object()->do_glance(this_object()->query_option(OPT_BRIEF));
    }

    /* See is people were hunting us or if we were hunting people. */
    this_object()->adjust_combat_on_move(0);

    dragged = query_prop(TEMP_DRAGGED_ENEMIES);

    if (pointerp(dragged))
        dragged = filter(dragged, #'objectp);
    else
        dragged = ({});

    if (sizeof(dragged))
    {
    	foreach(object dragee: dragged)
        {
            tell_room(environment(dragee), QCTNAME(dragee) +
                " leaves following " + QTNAME(this_object()) + ".\n", ({dragee}));
            dragee->move_living("M", to_dest);
            tell_room(environment(dragee), QCTNAME(dragee) +
                " arrives following " + QTNAME(this_object()) + ".\n",
                ({ dragee, this_object() }) );
            tell_object(this_object(), ({string}) dragee->query_The_name(this_object()) +
                " arrives following you.\n");
        }
        remove_prop(TEMP_DRAGGED_ENEMIES);
    }

    if (!dont_follow &&
        stringp(how) &&
        (size = sizeof(team = query_team())))
    {
        /* Command for the followers if this is a leader. */
        if (!sizeof(verb))
        {
            if (sizeof(explode(how, " ")) == 1)
            {
                com = how;
            }
            else
            {
                com = "";
            }
        }
        else if (com = ({string}) env->query_dircmd())
        {
            com = verb + " " + com;
        }
        else
        {
            com = verb;
        }

        /* Move the present team members. */
        index = -1;

        while(++index < size)
        {
            if ((environment(team[index]) == env) &&
                ({int}) this_object()->check_seen(team[index]))
            {
                team[index]->follow_leader(com);
            }
        }
    }

    /* Only reset this_player() if we weren't this_player already. */
    if (oldtp != this_object())
    {
        log_debug("Move oldtp != this_object()");
        set_this_player(oldtp);
    }

    log_debug("Move success");
    return 0;
}


/*
 * Function name: follow_leader
 * Description  : If the leader of the team moved, follow him/her.
 * Arguments    : string com - the command to use to follow the leader.
 *
 * WARNING      : This function makes the person command him/herself. This
 *                means that when a wizard is in a team, the team leader can
 *                force the wizard to perform non-protected commands. Wizard
 *                commands cannot be forced as they are protected.
 */
public void follow_leader(string com)
{
    /* Only accept this call if we are called from our team-leader. */
    if (previous_object() != query_leader())
    {
        return;
    }

    set_this_player(this_object());

    /* We use a call_other since you are always allowed to force yourself.
     * That way, we will always be able to follow our leader.
     */
    this_object()->command("$" + com);
}


/*
 * Function name: reveal_me
 * Description  : Reveal me unintentionally.
 * Arguments    : int tellme - true if we should tell the player.
 * Returns      : int - 1 : He was hidden, 0: He was already visible.
 */
public nomask int reveal_me(int tellme)
{
    object *list, tp;
    int index, size;

    if (!query_prop(OBJ_I_HIDE))
    {
        return 0;
    }

    if (tellme)
    {
        this_object()->catch_msg("You are no longer hidden.\n");
    }

    list = FILTER_LIVE(all_inventory(environment()) - ({ this_object() }) );
    remove_prop(OBJ_I_HIDE);

    tp = this_player();
    set_this_player(this_object());

    index = -1;
    size = sizeof(list);

    while(++index < size)
    {
        list[index]->combat_init();
        list[index]->init_attack();

        if (check_seen(list[index]))
        {
            tell_object(list[index],
                ({string}) this_object()->query_The_name(list[index]) +
                " decides to come out of hiding.\n");
        }
        else
        {
            tell_object(list[index], "You are startled to find " +
                ({string}) this_object()->query_art_name(list[index]) +
                " suddenly standing next to you!\n");
        }
    }

    set_this_player(tp);

    return 1;
}
