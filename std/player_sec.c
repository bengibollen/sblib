/*
 * /std/player_sec.c
 *
 * This file is statically inherited by player_pub to ensure the
 * protection of all lower level routines.
 */

#pragma strict_types

inherit "/std/living";

/* This order is on purpose to limit the number of prototypes necessary. */
#include "/std/player/savevars_sec.c"
#include "/std/player/quicktyper.c"
#include "/std/player/cmd_sec.c"
#include "/std/player/getmsg_sec.c"
#include "/std/player/death_sec.c"
#include "/std/player/querys_sec.c"
#include "/std/player/pcombat.c"
#include "/std/player/more.c"

#include <files.h>
#include <formulas.h>
#include <language.h>
#ifndef OWN_STATUE
#include <living_desc.h>
#endif
#include <macros.h>
#include <mail.h>
#include <money.h>
#include <options.h>
#include <ss_types.h>
#include <std.h>
#include <stdproperties.h>

#define LINKDEATH_TIME    180 /* three minutes */
#define AUTOLOAD_INTERVAL 0
#define RECOVERY_INTERVAL 1

/* 
 * List of properties that are to be saved in the player object. This list
 * is used at both restore and save since the name of the property itself
 * isn't stored, just the value.
 * 
 * WARNING: If you add something, add it at the end of the array. Do
 *          NOT insert anything, or all previously stored arrays will
 *          be restored in an incorrect sequence.
 */
#define SAVE_PROPS ({ CONT_I_WEIGHT, CONT_I_HEIGHT, WIZARD_I_BUSY_LEVEL, \
                      PLAYER_I_MORE_LEN, CONT_I_VOLUME, LIVE_I_LANGUAGE })

/*
 * Global variables. They are not saved.
 */
#ifndef NO_SKILL_DECAY
private static int do_skill_decay = 0; /* Flag to control skill decay */
#endif


/*
 * Function name: query_def_start
 * Description  : Return the default starting location of the player type.
 *                This function is supposed to be replaced in inheriting
 *                player objects.
 */
public string query_def_start()
{
    return DEFAULT_START;
}


/*
 * Function name: query_orig_stat
 * Description:   Return the default starting stats of a player
 *                This function is supposed to be replaced in inheriting
 *                player objects.
 */
public int *query_orig_stat() 
{
    int i, *list;

    list = ({});
    i = -1;
    while(++i < SS_NO_STATS)
    {
        list += ({ 1 });
    }
    return list;
}


/*
 * Function name: query_orig_learn
 * Description:   Return the default starting stats of a player
 *                This function is supposed to be replaced in inheriting
 *                player objects.
 */
public int *query_orig_learn() 
{
    int i, *list;

    list = ({});
    i = -1;
    while(++i < SS_NO_EXP_STATS)
    {
        list += ({ 100 / SS_NO_EXP_STATS });
    }
    return list;
}


#ifndef NO_ALIGN_TITLE
/*
 * Function name: query_new_al_title
 * Description:   Return the default starting title of a player
 *                This function is supposed to be replaced in inheriting
 *                player objects.
 */
public string query_new_al_title()
{
    return "neutral";
}
#endif 


/*
 * Function name: fixup_screen
 * Description:   Restore the players screen width. Normally called
 *                during login.
 */
public nomask void fixup_screen()
{
    int width = query_option(OPT_SCREEN_WIDTH);

    /* Value 0 means unset, so default to 80. */
    if (!width)
    {
        width = 80;
    }
    /* Value -1 means no screen width, ergo no wrapping. */
    if (width == -1)
    {
        width = 0;
    }

//  set_screen_width(width);
}


#ifndef NO_SKILL_DECAY
/*
 * Function name:   query_skill_decay
 * Description:     Gives back the skill decay status
 * Returns:         The skill decay status
 */
public nomask int query_skill_decay()
{
    return do_skill_decay;
}


/*
 * Function name: get_train_max
 * Description:   Return the max value of a skill that a trainer trains.
 * Arguments:     skill - the skill to be examined.
 *                ob - the object defining the skill
 * Returns:       See above.
 */
static nomask int get_train_max(int skill, mixed ob)
{
    int rval = 0;

#ifdef LOG_BAD_TRAIN
    if (catch(rval = ({int}) ob->sk_query_max(skill, 1)))
        log_file(LOG_BAD_TRAIN, ctime(time()) + ": " + ob + "\n");
#else
    catch(rval = ({int}) ob->sk_query_max(skill, 1));
#endif 

    return rval;
}


/*
 * Function name: query_decay_skill
 * Description:   Return 1 if a skill should be decayed, 0 if not.
 * Arguments:     list - the list of objects defining the skill train max
 *                skill - the skill to be examined.
 * Returns:       See above.
 */
static nomask int query_decay_skill(mixed list, int skill)
{
    int *sklist;
    int index;
    int size;
    int maximum;
    int sk;

    /* Load all trainers first */
    catch(list->teleledningsanka());

    /* Check the contents */
    sklist = ({ }) + map(list, (: get_train_max(skill, $1) :));
    sk = (sizeof(SS_SKILL_DESC[skill]) ? SS_SKILL_DESC[skill][4] : 0);
    sklist += ({ ((sk > MIN_SKILL_LEVEL) ? sk : MIN_SKILL_LEVEL) });

    maximum = max(sklist);

    return (query_base_skill(skill) > maximum);
}


/*
 * Function name: decay_skills
 * Description:   Do skill decay in the player
 *                Call this function ONLY if it's necessary, as when
 *                entering the game or entering/leaving a guild as
 *                it's a bit costly.
 */
static nomask void decay_skills()
{
    mixed obs;
    mixed otmp;
    int *skills, i, sz;
    string str, tmp;
    
    /* Only do this on the proper interval, and wizards pass by, of course */
    if ((query_decay_time() < SKILL_DECAY_INTERVAL) ||
        query_wiz_level())
    {
        return;
    }

    set_this_player(this_object());
    
    /* Reset the time for next call. */
    reset_decay_time();

    /* Get the list of trainer objects */
    obs = ({});
    otmp = ({mixed}) this_object()->query_guild_trainer_occ();
    obs += pointerp(otmp) ? otmp : ({ otmp });
    otmp = ({mixed}) this_object()->query_guild_trainer_race();
    obs += pointerp(otmp) ? otmp : ({ otmp });
    otmp = ({mixed}) this_object()->query_guild_trainer_lay();
    obs += pointerp(otmp) ? otmp : ({ otmp });
    otmp = ({mixed}) this_object()->query_guild_trainer_craft();
    obs += pointerp(otmp) ? otmp : ({ otmp });
    obs -= ({ 0 });
    
    /* Filter all relevant skills */
    skills = filter(query_all_skill_types(), (: 99999 > $1 :));

    /* Find out what skills need decay */
    skills = filter(skills, (: query_decay_skill(obs, $1) :));

    /* Do decay */
    if (sizeof(skills))
    {
        tmp = ((tmp = ({string}) this_object()->query_guild_name_occ()) ? tmp : "") + ", " +
            ((tmp = ({string}) this_object()->query_guild_name_lay()) ? tmp : "") + ", " +
	    ((tmp = ({string}) this_object()->query_guild_name_craft()) ? tmp : "") + ", " +
            ((tmp = ({string}) this_object()->query_guild_name_race()) ? tmp : "");
        
        str = sprintf("%s\t\t%s\n%s\t\t", ({string}) this_object()->query_name(), tmp,
            ctime(time()));

        sz = sizeof(skills);
        for (i = 0; i < sz; i++)
        {
            str += sprintf("%i ", skills[i]);
            set_skill(skills[i], query_base_skill(skills[i]) - 1);
        }
        log_file("DECAY_LOG", str + "\n", 50000);
    }
    else
    {
        do_skill_decay = 0;
    }
}


/*
 * Function name:   setup_skill_decay()
 * Description:     setup the skill decay flag.
 */
public nomask void setup_skill_decay()
{
    if (query_wiz_level())
        return;

    do_skill_decay = 1;

    call_out(#'decay_skills, 90);
}
#endif


/*
 * Function name: reset_userids
 * Description  : Called to set the euid of this player. Wizards get their own
 *                name as effective user id.
 */
public void reset_userids()
{
    
    log_debug("Resetting user IDs for player.");

    configure_object(this_object(), OC_EUID, 0);

    if (({int}) SECURITY->query_wiz_rank(query_real_name()))
    {
        SECURITY->reset_wiz_uid(this_object());
    }

    configure_object(this_object(), OC_EUID, query_real_name());
    log_debug("User IDs reset successfully.");
}


/*
 * Function:     new_init
 * Description:  Initialises all variables to default conditions.
 */
static nomask void new_init()
{
    int i;
    int *ostat;

    log_debug("Initializing player statistics.");

    ostat = query_orig_stat();
    log_debug("Original stats: %O", ostat);

    i = -1;
    while(++i < SS_NO_EXP_STATS)
    {
        log_debug("Setting exp stat %d to %d", i, ostat[i]);
        set_base_stat(i, ostat[i]);
    }

    stats_to_acc_exp();

    set_learn_pref(query_orig_learn());

#ifndef NO_ALIGN_TITLE
    set_al_title(query_new_al_title());
#endif
}


/*
 * Function name: slow_load_auto_files
 * Description  : Loads one autoloaded object. We use an alarm to make sure
 *                that people always get their stuff, even when they carry too
 *                much for their own good.
 * Arguments    : string *auto_files - the autoloading files still to load.
 */
nomask static void slow_load_auto_files(string *auto_files)
{
    string file;
    string argument;
    object ob;

    if (sizeof(auto_files) > 1)
    {
        call_out((: slow_load_auto_files(auto_files[1..]) :), 0);
    }
    else
    {
        remove_prop(PLAYER_I_AUTOLOAD_TIME);
    }

    set_this_player(this_object());

    if (sscanf(auto_files[0], "%s:%s", file, argument) != 2)
    {
        file = auto_files[0];
        argument = 0;
    }

    catch(ob = clone_object(file));
    if (!objectp(ob))
    {
        return;
    }

    /* Note that we don't check for sizeof() since we also want to call
     * init_arg() if the format is 'filename:'.
     */
    if (stringp(argument))
    {
        if (({int}) ob->init_arg(argument))
        {
            ob->remove_object();
            return;
        }
    }
    ob->move(this_object(), 1);
}


/*
 * Function name: load_auto_files
 * Description  : Loads all autoloaded objects. We use an alarm to make sure
 *                that people always get their stuff, even when they carry too
 *                much for their own good.
 */
nomask static void load_auto_files()
{
    string *auto_files;

    auto_files = query_auto_load();

    if (!sizeof(auto_files))
    {
        return;
    }

    add_prop(PLAYER_I_AUTOLOAD_TIME, (time() + 5 + sizeof(auto_files)));

    call_out((: slow_load_auto_files(auto_files) :), AUTOLOAD_INTERVAL);
}


/*
 * Function name: slow_load_recover_files
 * Description  : Loads one recoverable object. We use an alarm to make sure
 *                that people always get their stuff, even when they carry too
 *                much for their own good.
 * Arguments    : string *recover_files - the recoverable files still to load.
 */
nomask static void slow_load_recover_files(string *recover_files)
{
    string  file;
    string  argument;
    object  ob;

    log_debug("Loading recover files...");
    if (sizeof(recover_files) > 1)
    {
        call_out((: slow_load_recover_files(recover_files[1..]) :), RECOVERY_INTERVAL);
    }

    set_this_player(this_object());

    if (sscanf(recover_files[0], "%s:%s", file, argument) != 2)
    {
        file = recover_files[0];
        argument = 0;
    }

    catch(ob = clone_object(file));
    if (!objectp(ob))
    {
        return;
    }

    /* Note that we don't check for sizeof() since we also want to call
     * init_recover() if the format is 'filename:'.
     */
    if (stringp(argument))
    {
        if (({int}) ob->init_recover(argument))
        {
            ob->remove_object();
            return;
        }
    }

    /* Tell the person. Little touch to tell when it's the last one. */
    write("You " + ((sizeof(recover_files) == 1) ? "finally " : "") +
        "recover your " + ({string}) ob->short() + ".\n");
    ob->move(this_object(), 1);
}


/*
 * Function name: load_recover_files
 * Description  : Loads all recoverable objects. We use an alarm to make sure
 *                that people always get their stuff, even when they carry too
 *                much for their own good.
 */
nomask static void load_recover_files()
{
    string *recover_files;
    int     size;

    recover_files = query_recover_list();

    /* Reset the recovery list. */
    set_recover_list( ({ }) );

    if (!(size = sizeof(recover_files)))
    {
        return;
    }

    /* Do not restore after a certain time. */
    if (time() - query_logout_time() > F_RECOVERY_LIMIT)
    {
        return;
    }

    catch_tell("Preparing to recover " + LANG_WNUM(size) + " item" +
        ((size == 1) ? "" : "s") + ".\n");

    call_out((: slow_load_recover_files(recover_files) :), RECOVERY_INTERVAL);
}


/*
 * Function name: load_auto_shadows
 * Description  : This function loads and initialises all shadows that the
 *                player should have when he logs in. No special measures are
 *                taken for shadows at login time.
 */
nomask static void load_auto_shadows()
{
    string *load_arr;
    string file;
    string argument;
    object ob;
    int    index;
    int    size;

    load_arr = query_autoshadow_list();
    if (!sizeof(load_arr))
    {
        return;
    }

    index = -1;
    size = sizeof(load_arr);
    while(++index < size)
    {
        if (sscanf(load_arr[index], "%s:%s", file, argument) != 2)
        {
            write("Shadow load string corrupt: " + load_arr[index] + "\n");
            continue;
        }
        if (LOAD_ERR(file) ||
            !objectp(ob = find_object(file)))
        {
            write("Shadow not available: " + file + "\n");
            continue;
        }

        catch(ob = clone_object(file));
        if (argument)
        {
            catch(ob->autoload_shadow(argument));
        }
        else
        {
            catch(ob->autoload_shadow(0));
        }
    }
}


/*
 * Function name: init_saved_props
 * Description  : Add the saved properties to the player.
 */
static void init_saved_props()
{
    int index = -1;
    int size = ((sizeof(SAVE_PROPS) < sizeof(saved_props)) ?
        sizeof(SAVE_PROPS) : sizeof(saved_props));

    while(++index < size)
    {
        if (saved_props[index])
        {
            add_prop(SAVE_PROPS[index], saved_props[index]);
        }
        else
        {
            remove_prop(SAVE_PROPS[index]);
        }
    }

    saved_props = 0;
}


/*
 * Function name: setup_player
 * Description:   Restore player variables from the player file and go through
 *                startup routines.
 * Arguments:     (string) pl_name - The player's name
 * Returns:       True if setup completed normally
 */
private nomask int setup_player(string pl_name)
{
    string      *souls;
    int         il, size;

    
    log_debug("Setting up player with name: " + pl_name);

    ::set_adj(({}));            /* No adjectives and no default */

    configure_object(this_object(), OC_EUID, 0);
    if (!({int}) SECURITY->load_player())
    {
        log_error("Failed to load player: " + pl_name);
        return 0;
    }
    configure_object(this_object(), OC_EUID, query_real_name());

    reset_userids();

    set_adj(0);                 /* Set the adjectives as loaded */
    fixup_screen();

    /* Mortals should have one of the base races by default. */
    if (!query_wiz_level())
    {
        
        log_debug("Resetting race name for mortal player: " + to_string(this_object()));
        reset_race_name();
    }

    add_name(query_race_name());
    add_pname(LANG_PWORD(query_race_name()));

    log_debug("Adding names for player: " + query_name());

    if (query_wiz_level())
    {
        
        log_debug("Adding wizard names for player: " + query_name());
        /* Wizards should have the term wizard as added name. */
        add_name("wizard");
        add_pname("wizards");
    }
    else
    {
        log_debug("Resetting entrance messages for mortal player: " + to_string(this_object()));
        /* Mortals should not have altered entrance messages. */
        move_reset();
    }

    /* Restore the bits */
    unpack_bits();

    /* Make some sanity things to guard against old and patched .o files */
    set_learn_pref(query_learn_pref(-1));

    set_living_name(pl_name);
    cmd_sec_reset();
    player_save_vars_reset();

    /* Check the accumulated experience and then set the stats. */
    check_acc_exp();
    acc_exp_to_stats();

    /* Reset the restriction. */
    restricted = 0;

    /* Restore the saved properties and add a default one. */
    add_prop(PLAYER_I_MORE_LEN, 20);
    init_saved_props();

    /* Restore the whimpy option into the internal variable. */
    ::set_whimpy(query_option(OPT_WHIMPY));

    /* Non wizards should not have a lot of souls */
    if (!query_wiz_level())
    {
        // log_debug("Querying command souls for player: " + to_string(this_object()));
        souls = query_cmdsoul_list();
        if (sizeof(souls))
        {
            il = -1;
            size = sizeof(souls);
            while(++il < size)
            {
                // log_debug("Removing command soul: " + souls[il]);
                remove_cmdsoul(souls[il]);
            }
        }

        souls = query_tool_list();
        if (sizeof(souls))
        {
            il = -1;
            size = sizeof(souls);
            while(++il < size)
            {
                // log_debug("Removing tool soul: " + souls[il]);
                this_object()->remove_toolsoul(souls[il]);
            }
        }
    }

    if (!m_alias_list)
    {
        m_alias_list = ([ ]);
    }
    if (!m_nick_list)               
    { 
        m_nick_list = ([ ]);   
    } 

    /* Get the autoloading shadows and the autoloading objects. Start the
     * recovery with a little alarm to make it safe. */
    load_auto_shadows();
    load_auto_files();
    call_out(#'load_recover_files, RECOVERY_INTERVAL);

    /* Set up skill decay now that the guild shadows are loaded. Do a first
     * decay as well, making it a bit more frequent for people who log
     * on/off/on all the time.
     */
#ifndef NO_SKILL_DECAY
    decay_time = time();
    setup_skill_decay();
#endif
    // Combat object already configure in create_living()
    // log_debug("Configuring combat object for player");
    // query_combat_object()->cb_configure();
    // log_debug("Player setup completed successfully.");
    return 1;
}


#ifdef CHANGE_PLAYEROB_OBJECT
/*
 * Function name: change_player_object
 * Description:   Initialize this player object based on an existing one.
 * Arguments:     (object) old_plob - the player object to be used to initialize
 *                                   this one.
 * Returns:       True if initialization was successful
 */
public nomask int change_player_object(object old_plob)
{

    if (MASTER_OB(previous_object()) != CHANGE_PLAYEROB_OBJECT)
    {
        return 0;
    }

    // log_debug("Changing player object for: " + old_plob->query_real_name());

    set_name(old_plob->query_real_name());

    setup_player(old_plob->query_real_name());

    // log_debug("Player object changed successfully for: " + old_plob->query_real_name());
    this_object()->start_player();

    add_prop(PLAYER_I_LASTXP, old_plob->query_prop(PLAYER_I_LASTXP));
    
    // log_debug("Player object initialization complete for: " + old_plob->query_real_name());
    return 1;
}
#endif


/*
 * Function name: try_start_location
 * Description  : Attempt to make the player start in a start location.
 * Arguments    : string path - the path to try.
 * Returns      : int 1/0 - if true, the player moved to the room.
 */
static nomask int try_start_location(string path)
{
    object room;

    
    // log_debug("Attempting to check start location: " + path);

    /* Sanity check. */
    if (!sizeof(path) ||
        file_size(path + ".c") <= 0)
    {
        return 0;
    }

    /* Try to load the room, and then try to move the player. */
    LOAD_ERR(path);
    if (objectp(room = find_object(path)))
    {
        catch(move_living(0, room));
    }

    /* Return true if we moved into a room. */
    return objectp(environment());
}


/*
 * Function name: enter_game
 * Description  : Enter the player into the game.
 * Arguments    : string pl_name - the name of the player.
 *                string pwd     - the password if it was changed.
 * Returns      : int 1/0 - login succeeded/failed.
 */
public nomask int enter_game(string pl_name, string pwd)
{
    int    lost_money;
    string path;
    object room;

    
    // log_debug("Enter game and checking player setup...");
    // log_debug("Player name: " + pl_name);
    // log_debug("Saved password: %s", to_string(password));
    // log_debug("Password: " + pwd);
    configure_interactive(this_object(), IC_ENCODING, "utf-8");

    if ((MASTER_OB(previous_object()) != LOGIN_OBJECT) &&
        (MASTER_OB(previous_object()) != LOGIN_NEW_PLAYER))
    {
        write("Bad login object: " + object_name(previous_object()) + "\n");
        log_error("Bad login object: " + object_name(previous_object()));
        return 0;
    }

    set_name(pl_name);
    cmdhooks_reset();
    // log_debug("(cmdhooks_reset) Current actions: %O", query_actions(this_object()));
    setup_player(pl_name);
    // log_debug("(setup_player) Current actions: %O", query_actions(this_object()));

    /* Tell the player when he was last logged in and from which site. */
    if (MASTER_OB(previous_object()) == LOGIN_OBJECT)
    {
        write("Last login at  : " + ctime(query_login_time()) +
            "\nLast login from: " + query_login_from() + "\n");
    }

    /* Try the temporary start location. */
    if (query_temp_start_location() &&
        ({int}) SECURITY->check_temp_start_loc(query_temp_start_location()) >= 0)
    {
        try_start_location(query_temp_start_location());
        set_temp_start_location(0);
    }

    /* Try the default start location if necessary. */
    if (!environment())
    {
        if (!query_default_start_location() ||
            (!query_wiz_level() && 
             (({int}) SECURITY->check_def_start_loc(query_default_start_location()) < 0)))
        {
            set_default_start_location(query_def_start());
        }
        try_start_location(query_default_start_location());
    }

    /* Default start location failed. Try the racial default. */
    if (!environment())
    {
        write("\nSERIOUS PROBLEM with your start location.\n" +
            "You revert to your default racial start location.\n\n");
        set_default_start_location(query_def_start());
        try_start_location(query_def_start());
    }

    /* Eeks! Racial default start location failed. */
    if (!environment())
    {
        write("\nSERIOUS PROBLEM!\nYou are starting in the void!\n" +
            "Report this to a knowledgeble wizard at once!\n\n");
    }

    /* Start him up */
    this_object()->start_player();

    /* Do this after startup, so we can use the address and time at startup. */
    set_login_time();
    set_login_from();

    /* Initialise the experience counter. */
    add_prop(PLAYER_I_LASTXP, query_exp());

    /* Let players start even if their start location is bad */
    if (!environment() &&
        !query_wiz_level())
    {
        if (catch(move_living(0, query_def_start())))
        {
            /* If this start location is corrupt too, destruct the player */
            write("PANIC, your starting locations are corrupt!!\n");
            log_error("PANIC, your starting locations are corrupt for: %s", pl_name);;
            destruct(this_object());
        }
    }

    if ((lost_money = query_tot_value()) > 0)
    {
        tell_object(this_object(), "You find " + lost_money +
            " coppers of your lost money.\n");
        MONEY_ADD(this_object(), lost_money);
    }

    /* If a password was changed, set it. */
    if (sizeof(pwd))
    {
        set_password(pwd);
        save_me(1);
    }

    return 1;
}


/*
 * Function name: open_player
 * Description  : This function may only be called by SECURITY or by the
 *                login object to reset the euid of this object.
 */
public nomask void open_player()
{
    
    log_debug("Opening player object.");

    if ((previous_object() == find_object(SECURITY)) ||
        (MASTER_OB(previous_object()) == LOGIN_OBJECT))
    {
        configure_object(this_object(), OC_EUID, query_real_name());
        log_debug("Player object opened with euid set to 0.");
    }

    log_debug("Player object opened successfully.");
}


/*
 * Function name: fix_saveprops_list
 * Description  : Before the player is saved, this function is called to
 *                store several properties into an array that will be
 *                saved in the player file.
 */
nomask public void fix_saveprop_list()
{
    int i, size;

    /* Fix the saved_props list before save */
    saved_props = ({ });
    i = -1;
    size = sizeof(SAVE_PROPS);
    while(++i < size)
    {
        saved_props += ({ query_prop(SAVE_PROPS[i]) });
    }
}


/*
 * Function name: save_player
 * Description  : This function actually saves the player object.
 * Arguments    : string pl_name - the name of the player
 * Returns      : int 1/0 - success/failure.
 */
nomask public int save_player(string pl_name)
{
    if (!pl_name)
    {
        return 0;
    }
    log_debug("This object: %O", this_object());
    log_debug("Saving player file: " + pl_name);
    log_debug("Uid: %s", getuid());
    log_debug("Euid: %s", to_string(geteuid(this_object())));
    log_debug("Previous object: %O", previous_object());
    log_debug("Previous object uid: %s", getuid(previous_object()));
    log_debug("Previous object euid: %s", geteuid(previous_object()));

    pack_bits();
    configure_object(this_object(), OC_EUID, getuid(previous_object()));
    // log_debug("Savedata: %O", save_object());
    save_object(PLAYER_FILE(pl_name));
    configure_object(this_object(), OC_EUID, query_name());

    /* Discard the props again */
    saved_props = 0;
    return 1;
}


/*
 * Function name: load_player
 * Description  : This function actually loads the player file into the
 *                player object.
 * Arguments    : string pl_name - the name of the player.
 * Returns      : int 1/0 - success/failure.
 */
nomask public int load_player(string pl_name)
{
    int ret;
    string file;
   
    if (!pl_name)
    {
        return 0;
    }

    log_debug("Loading player file: " + pl_name);
    // log_debug("This object: %O", this_object());

    configure_object(this_object(), OC_EUID, getuid(previous_object()));

    if (file_size(PLAYER_FILE(pl_name) + ".o") <= 0)
    {
        log_error("Player file not found: %s", pl_name);
        return 0;
    }

    // log_debug("Player file contents:\n%s", read_file(PLAYER_FILE(pl_name) + ".o"));

    ret = restore_object(PLAYER_FILE(pl_name));

    if (!ret)
    {
        log_error("Failed to restore player file: %s", PLAYER_FILE(pl_name));
    }

    configure_object(this_object(), OC_EUID, getuid());
    return ret;
}


/*
 * Function name: linkdeath_hook
 * Description  : This routine is called when the player linkdies. Do not mask
 *                it unless you really need to.
 * Arguments    : int linkdeath - 1/0 - if true, the player linkdied, else
 *                    he revives from linkdeath.
 */
public void linkdeath_hook(int linkdeath)
{
}


/*
 * Function name: actual_linkdeath
 * Description  : This function is called when the player actually linkdies.
 *                If the player is in combat, this will be delayed, or else
 *                it is called directly.
 */
static nomask void actual_linkdeath()
{
#ifdef STATUE_WHEN_LINKDEAD
#ifdef OWN_STATUE
    OWN_STATUE->linkdie(this_object());
#else   
    tell_room(environment(), LD_STATUE_TURN(this_object()), ({ }) );
#endif
#endif

    /* People should not autosave while they are linkdead. */
    stop_autosave();

    if (find_call_out(#'actual_linkdeath) != -1)
    {
        SECURITY->notify(this_object(), 5);
        remove_call_out(#'actual_linkdeath);
    }
    set_linkdead(1);

    /* Allow a shadow to take notice of the linkdeath. */
    this_object()->linkdeath_hook(1);
    /* Allow items in the top level inventory of the player to take notice
     * of the linkdeath.
     */
    all_inventory(this_object())->linkdeath_hook(this_object(), 1);
}


/*
 * Function name: linkdie
 * Description  : When a player linkdies, this function is called.
 */
nomask public void linkdie()
{
    if (previous_object() != find_object(SECURITY))
    {
        return;
    }

    if (query_relaxed_from_combat())
    {
        actual_linkdeath();
    }
    else
    {
#ifdef STATUE_WHEN_LINKDEAD
#ifdef OWN_STATUE
        OWN_STATUE->nonpresent_linkdie(this_object());
#endif
#endif

        tell_room(environment(), ({
            capitalize(query_real_name()) + " loses touch with reality.\n",
            "The " + query_nonmet_name() + " loses touch with reality.\n",
            "" }),
            ({ this_object() }) );

        call_out(#'actual_linkdeath, LINKDEATH_TIME);
    }
}


/*
 * Function name: query_linkdead_in_combat
 * Description  : This function returns true if the player is linkdead,
 *                but still in combat.
 * Returns      : int 1/0 - in combat while linkdead or not.
 */
nomask public int query_linkdead_in_combat()
{
    return (find_call_out(#'actual_linkdeath) != -1);
}


/*
 * Function name: revive
 * Description  : When a player revives from linkdeath, this function is
 *                called.
 */
nomask public void revive()
{
    if (MASTER_OB(previous_object()) != LOGIN_OBJECT)
    {
        return;
    }

    tell_object(this_object(), "You sense that you have " +
        MAIL_FLAGS[({int}) MAIL_CHECKER->query_mail(query_real_name())] + ".\n\n");
 
    /* If the player is not in combat, revive him. Else, just give a
     * a message about the fact that the player reconnected.
     */
    if (find_call_out(#'actual_linkdeath) == -1)
    {
        set_linkdead(0);

#ifdef OWN_STATUE
        OWN_STATUE->revive(this_object());
#else   
        tell_room(environment(), QCTNAME(this_object()) + " " +
            STATUE_TURNS_ALIVE + ".\n", ({ this_object() }) );
#endif

        /* We reset these variables so the player does not gain mana or
         * hitpoints while in LD.
         */
        player_save_vars_reset();
        save_vars_reset();

        /* Start autosaving again. */
        start_autosave();

        /* Allow a shadow to take notice of the revival. */
        this_object()->linkdeath_hook(0);
        /* Allow items in the top level inventory of the player to take notice
         * of the revival.
         */
        all_inventory(this_object())->linkdeath_hook(this_object(), 0);
    }
    else
    {
        tell_room(environment(), ({ capitalize(query_real_name()) +
            " gets in touch with reality again.\n",
            "The " + query_nonmet_name() +
            " gets in touch with reality again.\n",
            "" }),
            ({ this_object() }) );

#ifdef OWN_STATUE
        OWN_STATUE->nonpresent_revive(this_object());
#endif
        remove_call_out(#'actual_linkdeath);
    }
}


/*
 * Function name: linkdead_save_vars_reset
 * Description  : May be called externally while the player is linkdead to keep
 *                his save-vars reset, that is to prevent them from being
 *                updated during a save or quit action.
 */
public void linkdead_save_vars_reset()
{
    if (!interactive())
    {
	player_save_vars_reset();
	save_vars_reset();
    }
}


/*
 * Function name: new_save
 * Description  : This function is called to save the player initially.
 *                It is only called when new a player enters the game. It
 *                makes it possible to initialize variables using the
 *                standard set_ calls.
 * Arguments    : string pl_name - the name of the player.
 *                string pwd     - the (encrypted) password of the player.
 *                string pfile   - the player save file.
 * Returns      : int 1/0 - success/failure.
 */
public nomask int new_save(string pl_name, string pwd, string pfile)
{
    if (!CALL_BY(LOGIN_NEW_PLAYER))
    {
        return 0;
    }

    write("Creating new player: " + pl_name + "\n");
    configure_object(this_object(), OC_EUID, getuid());
    set_name(pl_name);
    set_password(pwd);
    set_player_file(pfile);
    new_init();                 /* Initialize all variables on startup */

    save_object(PLAYER_FILE(pl_name));
    return 1;
}


/*
 * Function name: create_living
 * Description  : Called to create the player. It initializes some variables.
 */
public nomask void create_living()
{
    log_debug("Creating player object: create_living.");
    player_save_vars_reset();
    new_init();                 /* All variables to default condition */
}


/*
 * Function name: reset_living
 * Description  : We don't want people to mask this function.
 */
public nomask void reset_living()
{
    return;
}


/*
 * Function name: command
 * Description  : Makes the player object execute a command, as if it was typed
 *                on the command line. For wizards, we have to test whether the
 *                euid of the caller allows to force the person.
 * Arguments    : string cmd - the command with arguments to perform. For players
 *                    this should always be prefixed with a "$".
 * Returns      : int - the amount of eval-cost ticks if the command was
 *                    successful, or 0 if unsuccessfull.
 */
public nomask int command(string cmd)
{

    log_debug("Executing command from player: %s", cmd);
    /* Test permissions if you try to force a wizard. */
    if (query_wiz_level() && objectp(previous_object()))
    {
        log_debug("Checking wiz force permissions for: %s", cmd);
        log_debug("Previous object: %O", previous_object());
        
        if (!({int}) SECURITY->wiz_force_check(geteuid(previous_object()), geteuid()))
        {
            log_debug("Wiz force check failed for: %s", cmd);
            return 0;
        }
    }

    /* Automatically add the "$" if it isn't added already to the command. This
     * to prevent people from using the quicktyper to circumvent being forced
     * to do particular commands.
     */
    if (cmd[0] != '$' && (previous_object() != this_object()))
    {
        cmd = "$" + cmd;
    }

    return ::command(cmd);
}


/*
 * Function name: id
 * Description  : Returns whether this object can be identified by a certain
 *                name. That isn't the case if the player hasn't met you
 *                while the real name is used.
 * Arguments    : string str - the name to test
 * Returns      : int 1/0 - true if the name is valid.
 */
public int id(string str)
{

    log_debug("Checking id for: " + str);
    if ((str == query_real_name()) &&
        notmet_me(this_player()))
    {
        log_debug("Player " + query_real_name() + " not met by " +
            ({string}) this_player()->query_real_name() + ".\n");
        return 0;
    }

    return ::id(str);
}


/*
 * Function name: parse_command_id_list
 * Description  : Mask of player_command_id_list() in /std/object.c to make sure
 *                that players cannot use the name of an NPC or player when that
 *                person hasn't been introduced to them.
 * Returns      : string * - the original parse_command_id_list() without the
 *                    lower case name of the person.
 */
public string *parse_command_id_list()         
{ 
    string *ids;

    ids = ::parse_command_id_list();

    if (sizeof(ids) &&
        notmet_me(this_player()))
    {
        ids -= ({ query_real_name() });
    }

    return ids;
}
