#pragma strict_types
#pragma no_inherit
#pragma warn_unused_variables
#pragma warn_dead_code
#pragma warn_applied_functions

inherit "/secure/simul/security";

/* All prototypes have been placed in /secure/master.h */
#include "/secure/master.h"

#include "/inc/config.h"
#include "/sys/configuration.h"  // For DC_* constants
#include "/sys/driver_hooks.h"    // For H_* constants
#include "/sys/interactive_info.h"
#include "/sys/erq.h"          // For ERQ_* constants
#include "/inc/macros.h"         // For ROOT_UID, BACKBONE_UID, etc.
#include "/inc/libfiles.h"          // For file_size, etc.
#include "/sys/files.h"          // For II_* constants
#include "/inc/std.h"          // For II_* constants
#include "/inc/libfiles.h"          // For file_size, etc.
#include "/sys/object_info.h"
#include "/sys/driver_info.h"
#include "/inc/cd_log.h"
#include "/sys/functionlist.h"
#include "/sys/lpctypes.h"

// Forward declarations for functions used in config


private nosave object logger;  // We'll initialize this later
/*
 * The global variables that are saved in the SAVEFILE.
 */
private int     game_started;
private string  *def_locations;
private string  *temp_locations;
// private mapping known_muds;
private int     runlevel;

/*
 * The global variables that are not saved.
 */
private static int     mem_fail_flag;
private static int     memory_limit;
private static mapping command_substitute = ([
    "n"  : "north",
    "s"  : "south",
    "w"  : "west",
    "e"  : "east",
    "u"  : "up",
    "d"  : "down",
    "sw" : "southwest",
    "se" : "southeast",
    "nw" : "northwest",
    "ne" : "northeast",
    ]);
private static mapping move_opposites;
// private static string  udp_manager;
private static int     irregular_uptime;


#include "/secure/master/error_handling.c"
#include "/secure/master/config.c"
#include "/secure/master/fob.c"
#include "/secure/master/siteban.c"
#include "/secure/master/spells.c"
#include "/secure/master/language.c"
#include "/secure/master/player.c"
#include "/secure/master/notify.c"
#include "/secure/master/sanction.c"
#include "/secure/master/guild.c"
#include "/secure/master/mail_admin.c"
#include "/secure/master/inaugurate_master.c"

#define DEBUG_RESTRICTED ( ({ "mudstatus", "swap", "shutdown", "send_udp" }) )

// Global variables
string mudlib_version = "SBLib v0.1";

// ---------- MANDATORY: Initialization Functions ----------
string get_master_uid()
{
    debug_message("Get master uid.\n");
    return ROOT_UID;
}


// For all logging calls, add error checking
private void log_message(string type, string msg, varargs mixed *args) 
{
    if (logger) {
        catch(logger->info(msg, args...));
    } else {
        debug_message(sprintf("%s: %s\n", type, sprintf(msg, args...)));
    }
}


// ---------- MANDATORY: Error Handling ----------
// void log_error(string file, string err, int warn, int line) {
//     log_message(warn ? "WARN" : "ERROR", "[%s:%d] %s", file, line, err);
// }

// void runtime_error(string err, string prog, string obj, int line, mixed culprit, int caught) {
//     logger->error("Runtime error in %s (%s) line %d: %s", 
//                  obj || "?", prog || "?", line, err);
// }

// ---------- MANDATORY: File Security ----------
// string valid_read(string path, string uid, string func, object|lwobject obj) {
//     return path;  // TODO: Implement proper security
// }

// string valid_write(string path, string uid, string func, object|lwobject ob) {
//     return path;  // TODO: Implement proper security
// }

// ---------- MANDATORY: File Resolution ----------
mixed inherit_file(string file, string compiled_file)
{
    if (file[0] != '/') file = "/" + file;
    return file;
}

    
mixed include_file(string file, string compiled_file, int sys_include)
{
    if (sys_include) {
        if (file_size("/inc/" + file) > 0)
            return "/inc/" + file;
        if (file_size("/sys/" + file) > 0)
            return "/sys/" + file;
    }
    string dir = compiled_file[..strrstr(compiled_file, "/")];
    if (file_size(dir + file) > 0)
        return dir + file;
    return 0;
}

// ---------- MANDATORY: Connection Handling ----------
private nosave mapping connection_throttle = ([]);
private nosave int cleanup_time;


public object connect()
{
    debug_message("\n=== New Connection Attempt ===\n");
    string ip = interactive_info(this_player(), II_IP_NUMBER);
    int now = time();
    
    logger->info("New connection attempt from %s", ip);
    
    if (now > cleanup_time) {
        logger->debug("Cleaning connection throttle map");
        connection_throttle = ([]);
        cleanup_time = now + 60;
    }
    
    if (member(({"127.0.0.1", "::1", "::ffff:127.0.0.1"}), ip) >= 0) {
        logger->info("Allowing local connection from %s", ip);
        return clone_object("/secure/login");
    }
    
    if (connection_throttle[ip] > 3) {
        logger->warn("Throttling connection from %s (too many attempts)", ip);
        return 0;
    }
    
    logger->info("Connection accepted from %s (attempt %d)", 
                ip, connection_throttle[ip] + 1);
    connection_throttle[ip] = connection_throttle[ip] + 1;
    return clone_object("/secure/login");
}


// ---------- MANDATORY: Root Access ----------
// string get_bb_uid() {
//     return "BACKBONE";
// }

// ---------- MANDATORY: UID Management ----------
string load_uid(string file)
{
    debug_message("Loading UID for file: " + file + "\n");
    return BACKBONE_UID;  // For now, everything gets BACKBONE_UID uid
}


string clone_uid(object blueprint, string name)
{
    debug_message("Cloning UID for file: " + name + "\n");
    return getuid(blueprint);  // For now, everything gets ROOT uid
}


string create_super(string file)
{
    debug_message("Creating super for file: " + file + "\n");
    return BACKBONE_UID;  // For now, everything gets BACKBONE_UID uid
}


string create_object(string file)
{
    debug_message("Creating object for file: " + file + "\n");
    return ROOT_UID;  // For now, everything gets ROOT uid
}


// ---------- Optional: Initialization ----------
// void flag(string arg) {
//     // Called for each -f flag passed to driver
// }

string *epilog(int eflag)
{
    return ({ });  // No objects to preload by default
}


void preload(string file)
{
    catch(load_object(file));
}


string get_simul_efun ()
// Load the simul_efun object(s) and return one or more paths of it.
{
    object ob;

    logger->info("Loading simul_efun");

    //error = catch(ob = load_object("/secure/simul_efun"));
    ob = load_object("/secure/simul_efun");
    if (objectp(ob))
    {
        ob->start_simul_efun();
        return "/secure/simul_efun";
    }
    logger->error("Failed to load /secure/simul_efun");

    efun::write("Failed to load " + "/secure/simul_efun" + ": ");
    efun::shutdown();
    return 0;
}


// ---------- Optional: Connection Handling ----------
void disconnect(object obj, string remaining)
{
    // Called when user disconnects
}


void remove_player(object user)
{
    // Called when player object is removed
}


void stale_erq(closure callback)
{
    // Called when ERQ connection is lost
}


// ---------- Optional: Runtime Support ----------
object compile_object(string filename)
{
    return 0;  // No virtual objects
}


string get_wiz_name(string file)
{
    return "NOBODY";
}


string printf_obj_name(object obj)
{
    return sprintf("<%s>", to_string(obj));
}


mixed prepare_destruct(object obj)
{
    return 0;  // Allow destruction
}


void quota_demon()
{
    // Memory quota handling
}


void receive_imp(string host, string msg, int port)
{
    // IMP message handling
}


void slow_shut_down(int minutes)
{
    // Gradual shutdown handling
}


void notify_shutdown()
{
    logger->info("System shutdown initiated");
}


// ---------- Optional: Error Handling ----------
// void dangling_lfun_closure() {
//     logger->warn("Dangling lfun closure detected");
// }

// mixed heart_beat_error(object culprit, string err, string prog, 
//                       string curobj, int line, int caught) {
//     logger->error("Heart beat error in %O: %s", culprit, err);
//     return 0;
// }

// ---------- Optional: Security ----------

// Validate the execution of a privileged operation.
//
// Arguments:
//   op   : the requestion operation (see below)
//   who  : the object requesting the operation (filename or object pointer)
//   arg  : additional argument, depending on <op>.
//   arg2 : additional argument, depending on <op>.
//
// Result:
//     >0: The caller is allowed for this operation.
//      0: The caller was probably misleaded; try to fix the error
//   else: A real privilege violation; handle it as error.
//
// Privileged operations are:
//   attach_erq_demon  : Attach the erq demon to object <arg> with flag <arg2>.
//   bind_lambda       : Bind a lambda-closure to object <arg>.
//   call_out_info     : Return an array with all call_out informations.
//   erq               : A the request <arg4> is to be send to the
//                       erq-demon by the object <who>.
//   execute_command   : Execute command string <arg2> for the object <arg>.
//   input_to          : Object <who> issues an 'ignore-bang'-input_to() for
//                       commandgiver <arg3>; the exakt flags are <arg4>.
//   nomask simul_efun : Attempt to get an efun <arg> via efun:: when it
//                       is shadowed by a 'nomask'-type simul_efun.
//   rename_object     : The current object <who> renames object <arg>
//                       to name <arg2>.
//   send_imp          : Send UDP-data to host arg3 (deprecated).
//   send_udp          : Send UDP-data to host <arg>.
//   set_auto_include_string : Set the string automatically included by
//                       the compiler (deprecated).
//   get_extra_wizinfo : Get the additional wiz-list info for wizard <arg>.
//   set_extra_wizinfo : Set the additional wiz-list info for wizard <arg>.
//   set_extra_wizinfo_size : Set the size of the additional wizard info
//                       in the wiz-list to <arg>.
//   set_driver_hook   : Set hook <arg> to <arg2>.
//   limited:          : Execute <arg> with reduced/changed limits.
//   set_limits        : Set limits to <arg>.
//   set_this_object   : Set this_object() to <arg>.
//   shadow_add_action : Add an action to function <arg4> of object <arg3>
//                       from the shadow <who> which is shadowing <arg3>.
//   symbol_variable   : Attempt to create symbol of a hidden variable
//                       of object <arg> with with index <arg2> in the
//                       objects variable table.
//   wizlist_info      : Return an array with all wiz-list information.
//
// call_out_info can return the arguments to functions and lambda closures
// to be called by call_out(); you should consider that read access to
// closures, mappings and pointers means write access and/or other privileges.
// wizlist_info() will return an array which holds, among others, the extra
// wizlist field. While a toplevel array, if found, will be copied, this does
// not apply to nested arrays or to any mappings. You might also have some
// sensitive closures there.
// send_udp() should be watched as it could be abused to mess up the IMP.
// The xxx_extra_wizinfo operations are necessary for a proper wizlist and
// should therefore be restricted to admins.
// All other operations are potential sources for direct security breaches -
// any use of them should be scrutinized closely.
int privilege_violation (string op, mixed who, mixed arg, mixed arg2, mixed arg3)
{
    /* This object and the simul_efun objects may do everything */
    if (who == this_object()
     || who == find_object(SIMUL_EFUN))
    {
        return 1;
    }

    switch(op) {
    case "bind_lambda":
        who = object_name(who);
    case "nomask simul_efun":
        if (objectp(who)) {
            who = object_name(who);
        }
      
        if (who[0..6] == "/secure") {
            return 1;
        }
        else {
            return -1;
        }
    case "call_out_info":
        return 1;
    case "rename_object":
    case "input_to":
        if (check_privilege(1)) {
            return 1;
        }
        else {
            logger->warn("Privilege violation for operation: " + op + ", who: " + to_string(who));
            return -1;
        }
    case "erq":
        if (objectp(who)) {
            who = object_name(who);
        }
      
        if (who[0..6] == "/secure") {
            return 1;
        }        
        switch(arg) {
            case ERQ_RLOOKUP:
                return 1;
            case ERQ_EXECUTE:
            case ERQ_FORK:
            case ERQ_AUTH:
            case ERQ_SPAWN:
            case ERQ_SEND:
            case ERQ_KILL:
            default:
                logger->warn("Privilege violation for operation: " + op + ", who: " + to_string(who));
                return -1;
        }
    case "configure_object":
    case "configure_interactive":
        return 1;
    
    default:
        return -1; /* Make this violation an error */
    }
}


// int query_allow_shadow(object victim) {
//     return 0;  // No shadowing by default
// }

// int valid_exec(string name, object ob, object obfrom) {
//     switch(name) {
//         case "secure/login.c":
//         case "secure/master.c":
//       if (interactive(obfrom) && !interactive(ob)) {
//            return 1;
//           }
//       }
//     return 0;  // No exec() by default
// }

int valid_query_snoop(object obj)
{
    return 0;  // No snoop queries by default
}


// int valid_snoop(object snoopee, object snooper) {
//     return 0;  // No snooping by default
// }

// ---------- Optional: Editor Support ----------
// string make_path_absolute(string str) {
//     return (str[0] == '/' ? str : "/" + str);
// }

// int save_ed_setup(object who, int code) {
//     return 0;  // No saved editor settings
// }

// int retrieve_ed_setup(object who) {
//     return 0;  // No saved editor settings
// }

// string get_ed_buffer_save_file_name(string file) {
//     return "/tmp/ed." + file;
// }

// ---------- Optional: parse_command Support ----------
// string *parse_command_id_list() {
//     return ({ });
// }

// string *parse_command_plural_id_list() {
//     return ({ });
// }

// string *parse_command_adjectiv_id_list() {
//     return ({ });
// }

// string *parse_command_prepos_list() {
//     return ({ });
// }

// string parse_command_all_word() {
//     return "all";
// }


/*
 * Function name: do_debug
 * Description  : This function is a front for the efun debug(). You are
 *                only allowed to call debug() through this object because we
 *                need to make some security checks.
 * Arguments    : string icmd - the debug command.
 *                mixed a1    - a possible argument to debug().
 *                mixed a2    - a possible argument to debug().
 *                mixed a3    - a possible argument to debug().
 * Returns      : mixed - the relevant return value for the particular
 *                        debug command.
 */
varargs mixed do_debug(string icmd, mixed a1, mixed a2, mixed a3)
{
    string euid = geteuid(previous_object());
    if (euid == BACKBONE_UID)
    {
        euid = ROOT_UID;
    }

    logger->debug(" === do_debug() ===\n");
    logger->debug("icmd: %s\n", icmd);
    logger->debug("euid: %s\n", euid);
    logger->debug("a1: %O\n", a1);
    logger->debug("a2: %O\n", a2);
    logger->debug("a3: %O\n", a3);
    logger->debug("previous_object: %O\n", previous_object());

    /* Some debug() commands are not meant to be called by just anybody. Only
     * 'root' and the administration may call them.
     */
    if ((icmd in DEBUG_RESTRICTED))
    {
        if ((euid != ROOT_UID) &&
            (previous_object() != this_object()) &&
            (previous_object() != find_object(SIMUL_EFUN))) //&&
//            (query_wiz_rank(euid) < WIZ_ARCH))
        {
            return 0;
        }
    }

    /* In order to get the variables of a certain object, you need to have
     * the proper euid. If you are allowed to write to the file, you are
     * also allowed to view its variables.
     */
    if ((icmd == "get_variables") &&
        (!valid_write(object_name(a1), euid, "get_variables", this_object())))
    {
        return 0;
    }

    /* Since debug() returns arrays and mappings by reference, we need to
     * process the value to make it secure, so people cannot alter it.
     */
    debug_message(sprintf("Debug command: %s\n", to_string(icmd)));
    switch (icmd)
    {
        case "get_variables":
            mixed *vars = variable_list(a1, RETURN_FUNCTION_NAME|RETURN_VARIABLE_VALUE);
            mapping var_map = ([]);
            if (pointerp(vars)) {
                for (int i = 0; i < sizeof(vars); i += 2) {
                    if (i + 1 < sizeof(vars)) {
                        var_map[vars[i]] = vars[i+1];
                    }
                }
            }
            return var_map; 
        default:
            break;

    }
//    return secure_var(debug(icmd, a1, a2, a3));
    return "";
}


/*
 * /secure/master.c
 *
 * This is the LPmud master object, used from version 3.0.
 *
 * It is the first object loaded, save the simul_efun object, but that
 * isn't a real object rather set of simulated efuns that have to be
 * somewhere.
 *
 * Everything written with 'write()' at startup will be printed on
 * stdout.
 *
 * 1. create() will be called first.
 * 2. flag() will be called once for every argument to the flag -f
 *         supplied to the driver.
 * 3. start_boot() will be called.
 * 4. preload_boot() will be called for each file to preload.
 * 5. final_boot() will be called.
 * 6. The game will enter multiuser mode, and enable log in.
 */



/*
 * This is the only object in the game in which we have to use absolute path'
 * to the inclusion files. The reason for this is that the function that
 * contains the search-path for inclusion files is defined in this object.
 */




// Then declare any critical functions needed before simul_efun loads
static void load_simul_efun()
{
    if (!find_object(SIMUL_EFUN)) {
        write("Loading simul_efun.\n");
        load_object(SIMUL_EFUN);
    }
}

/*
 * Function name: create
 * Description  : This is the first function called in this object.
 */
void create()
{
    "/lib/log.c"->debug("Creating master object.");

    /* Using a global variable for this is exactly TWICE as fast as using a
     * defined mapping. At this point we only have the default direction
     * commands here because they are all over the game in add_action()s.
     * The other abbreviations can easily be added to the respective souls.
     */
    command_substitute = ([
        "n"  : "north",
        "s"  : "south",
        "w"  : "west",
        "e"  : "east",
        "u"  : "up",
        "d"  : "down",
        "sw" : "southwest",
        "se" : "southeast",
        "nw" : "northwest",
        "ne" : "northeast",
        ]);

    move_opposites = ([
        "north"     : "the south",
        "south"     : "the north",
        "west"      : "the east",
        "east"      : "the west",
        "northwest" : "the southeast",
        "southwest" : "the northeast",
        "northeast" : "the southwest",
        "southeast" : "the northwest",
        "up"        : "below",
        "down"      : "above",
        "in"        : "outside",
        "out"       : "inside",
        ]);

    mem_fail_flag = 0;
#ifdef LARGE_MEMORY_LIMIT
    memory_limit = LARGE_MEMORY_LIMIT;
#else
    memory_limit = 28000000;
#endif
    configure_object(this_object(), OC_EUID, ROOT_UID);

    /* We reset the master every RESET time seconds, initially synchronizing
     * it at exactly 1 second after that occurance. I.e. if RESET_TIME is
     * 1 hour, it will be started exactly one second after the top of the
     * hour.
     */
    // set_alarm(((RESET_TIME + 1.0) - (to_float(time() % to_int(RESET_TIME)))),
    //     RESET_TIME, reset_master);

    /* Compute the uptime for this reboot. */
#ifdef REGULAR_UPTIME
    irregular_uptime = (REGULAR_UPTIME * 3600);
#ifdef UPTIME_VARIATION
    irregular_uptime +=
        (random(UPTIME_VARIATION * 3600) - (UPTIME_VARIATION * 1800));
#endif
#endif
    // game_started = 0;
    // start_boot(1); /* This does what we want */
    // game_started = 1;
}


/*
 * Function name: reset_master
 * Description  : This function will be called regularly, each RESET_TIME
 *                seconds. Since we want only one alarm running, from this
 *                function we can make calls to other modules that need it.
 */
static void reset_master()
{
    /* Check whether there is still enough memory to run the game. The
     * argument 1 means decay the domain experience.
     */
    check_memory(1);

    /* Gather information for the graph command. */
    probe_for_graph();

    /* Save the master. */
    save_master();
}


/*
 * Function name: short
 * Description  : This function returns the short description of this object.
 * Returns      : string - the short description.
 */
string short()
{
    return "the hole of the donut";
}


/*
 * Function name: save_master
 * Description  : This function saves the master object to a file.
 */
static void save_master()
{
    configure_object(this_object(), OC_EUID, ROOT_UID);

    save_object(SAVEFILE);
}


/*********************************************************************
 *
 * GD - INTERFACE LFUNS
 *
 * The below lfuns are called from the Gamedriver for various reasons.
 */

/*
 * Called on startup of game if '-f' is given on the commandline.
 *
 * To test a new function xx in object yy, do
 * driver "-fcall yy xx arg" "-fshutdown"
 */
static void flag(string str)
{
    string file, arg;

    if (game_started)
        return;

    if (sscanf(str, "for %d", arg) == 1)
    {
        return;
    }

    if (str == "shutdown")
    {
        do_debug("shutdown");
        return;
    }

    if (sscanf(str, "echo %s", arg) == 1)
    {
        write(arg + "\n");
        return;
    }

    if (sscanf(str, "call %s %s", file, arg) == 2)
    {
        arg = ({string})call_other(file, arg);
        write("Got " + arg + " back.\n");
        return;
    }
    write("master: Unknown flag " + str + "\n");
}


/*
 * Function name:   get_mud_name
 * Description:     Gives the name of the mud. The name will be #defined in
 *                  all files compiled in the mud. It can not contain spaces.
 * Returns:         Name of the mud.
 *                  We always return a string but we must declare it mixed
 *                  otherwise the type checker gets allergic reactions.
 */
mixed get_mud_name()
{
#ifdef MUD_NAME
    mixed n;

    n = MUD_NAME;
    if (mappingp(n))
    {
        n = n;
        // if (stringp(n[debug("mud_port")]))
        //     return n[debug("mud_port")];
        // else
        //     return n[0];
    }
    else if (stringp(MUD_NAME))
    {
        return MUD_NAME;
    }
#endif
//    return "LPmud(" + debug("version") + ":" + MUDLIB_VERSION + ")";
    return "SBMud";
}


/*
 * Function name: get_root_uid
 * Description  : Gives the uid of the root user.
 * Returns      : string - the name of the 'root' user.
 */
string get_root_uid()
{
    return ROOT_UID;
}


/*
 * Function name: get_bb_uid
 * Description  : Gives the uid of the backbone user. That is, each user
 *                that does not have an uid of its own. Apart from backbone
 *                all wizard names and domain names are valid uids. And
 *                root naturally.
 * Returns      : string - the name of the 'backbone' user.
 */
string get_bb_uid()
{
    return BACKBONE_UID;
}


/*
 * Function name: get_vbfc_object
 * Description  : This function returns the objectpointer to the VBFC
 *                object.
 * Returns      : object - the objectpointer to the VBFC object.
 */
object get_vbfc_object()
{
    return VBFC_OBJECT->ob_pointer();
}


/*
 * Function name: connect
 * Description  : This function is called every time a player connects. We
 *                return a clone of the login object through which the socket
 *                of the new player is connected.
 *                The efun input_to() cannot be called from here.
 * Returns      : object - the login object.
 */
// static object connect()
// {
//     write("\n");
//     configure_object(this_object(), OC_EUID, ROOT_UID);

//     return clone_object(LOGIN_OBJECT);
// }

/*
 * Function name: valid_set_auth
 * Description  : Whenever the hidden authorization information of an object,
 *                i.e. the uid or the euid, is altered this function is being
 *                called. It checks the format of the new autorization
 *                information and makes sure that the change is valid.
 * Arguments    : object setter      - the object forcing the change.
 *                object getting_set - the object being changed.
 *                string value       - the new value.
 * Returns      : string - the new value.
 */
string valid_set_auth(object setter, object getting_set, string value)
{
    string *oldauth;
    string *newauth;
    string auth = geteuid(getting_set);

    if (!stringp(value) ||
            ((setter != this_object()) &&
         (setter != find_object(SIMUL_EFUN))))
    {
        return auth;
    }

    newauth = explode(value, ":");
    if (sizeof(newauth) != 2)
    {
        return auth;
    }

    oldauth = (stringp(auth) ? explode(auth, ":") : ({ "0", "0"}) );
    if (newauth[0] == "#")
    {
        newauth[0] = oldauth[0];
    }
    if (newauth[1] == "#")
    {
        newauth[1] = oldauth[1];
    }

    return implode(newauth, ":");
}


/*
 * Function name: valid_seteuid
 * Description:   Checks if a certain user has the right to set a certain
 *                objects effective 'userid'. All objects has two 'uid'
 *                - Owner userid: Wizard who caused the creation.
 *                - Effective userid: Wizard responsible for the objects
 *                  actions.
 *                When an object creates a new object, the new objects
 *                'Owner userid' is set to creating objects 'Effective
 *                userid'.
 * Arguments:     ob:   Object to set 'effective' user id in.
 *                str:  The effuserid to be set.
 * Returns:       True if set is allowed.
 * Note:          Setting of effuserid to userid is allowed in the GD as
 *                well as setting effuserid to 0.
 */
int valid_seteuid(object ob, string str)
{
    string uid = getuid(ob);

    /* Root can be anyone it pleases. */
    if (uid == ROOT_UID)
    {
        return 1;
    }

    /* We can be ourselves. That is... set the euid to our uid. */
    if (uid == str)
    {
        return 1;
    }

    /* Arches and keepers can be anyone they please, except for root and
     * other arches or keepers.
     */
    if (query_wiz_rank(uid) >= WIZ_ARCH)
    {
        return ((query_wiz_rank(str) < WIZ_ARCH) && (str != ROOT_UID));
    }

    /* When arches and keepers are in a domain, the lord can't change
     * euid in his objects to the arch.
     */
    if (query_wiz_rank(str) >= WIZ_ARCH)
    {
        return 0;
    }

    /* A lord can be anyone of his subject wizards. */
    if ((uid == query_domain_lord(str)) ||
        (uid == query_domain_lord(query_wiz_dom(str))))
    {
        return 1;
    }

    /* No one else can be anything. */
    return 0;
}


/*
 * Function name: valid_write
 * Description  : Checks whether a certain user has the right to write a
 *                particular file.
 * Arguments    : string path  - the path name of the file to be write.
 *                mixed writer - the name or object of the writer.
 *                string func  - the calling function.
 * Returns      : int 1/0 - allowed/disallowed.
 */
mixed valid_write(string file, string uid, string func, object|lwobject writer)
{
    string *dirs, *wpath;
    string dname;
    string wname;
    string dir;
    int size;

    logger->debug("valid_write: %s %s %O", file, uid, writer);
    logger->debug("valid_write: %O", this_object());
    logger->debug("valid_write: %O", previous_object());

    if (objectp(writer))
    {
    	wpath = explode(object_name(writer), "/") - ({ "" });
    }

    /* Root may do as he please. */
    if (uid == ROOT_UID)
    {
        return 1;
    }

    /* Keepers and arches may do as they please. */
    if (query_wiz_rank(uid) >= WIZ_ARCH)
    {
        return 1;
    }

    /* Anonymous objects cannot do anything. */
    if (!(writer))
    {
        return 0;
    }

    dirs = explode(file, "/") - ({ "" });
    size = sizeof(dirs);

    switch(dirs[0])
    {
    case "open":
        /* Everyone may write in /open */
        return 1;

    case "d":
        /* Must be /d/Domain/something at least. */
        if (size < 3)
        {
            return 0;
        }

        /* The domain must be an existing domain. */
        dname = dirs[1];
        if (query_domain_number(dname) == -1)
        {
            return 0;
        }

	/* Check for special team directory here, as it is in the base
	 * domain, but not reachable by anyone (i.e. before any sanction
	 * checks are made)
	 */
        wname = dirs[2];
        dir = ((size > 3) ? dirs[3] : "");
	if ((dname == BASE_DOMAIN) && (wname == "ateam"))
	{
	    /* This can be allowed only if the wizard is a member of the team,
	     * or if the ateam code is writing in its own dir. Otherwise we
	     * disallow it.
	     */
	    return (dir in query_team_membership(uid) ||
		((dname == uid) && (sizeof(wpath) > 3) && (wpath[2] == "ateam") &&
		 (wpath[3] == dirs[3])));
	}

        /* The domain can write itself, unless it's the lonely wizard domain. */
        if (dname == uid)
        {
            return (dname != WIZARD_DOMAIN);
        }

        /* A Lord and steward can write anywhere in the domain. */
        if ((query_domain_lord(dname) == uid) ||
            (query_domain_steward(dname) == uid))
        {
            return 1;
        }

        /* We have to check for the directory sanctions here because they
         * might disclose the private directories.
         */
        if (recursive_valid_write_path_sanction(uid, dname, dirs[2..]))
        {
            return 1;
        }

        /* Only the Lord of a domain can write in the 'domain' directory,
         * except those with 'all' sanctions.
         */
        if (wname == "domain")
        {
            /* Except the private dir where only the liege/steward can write
             * and the subdir for restrictlogs where the mentor can write the
             * appropriate file
             */
            if (size > 5 &&
                (dirs[3] == "private" && dirs[4] == "restrictlog"))
            {
                return (dirs[5] in query_students(uid));
            }

            return valid_write_all_sanction(uid, dname);
        }

        /* Wizards can write everywhere in the domain unless this is the
         * domain for 'lonely' wizards.
         */
        if (query_wiz_dom(uid) == dname)
        {
            /* The exception being restricted wizards */
            if (query_restrict(uid) &
                (RESTRICT_RW_HOMEDIR | RESTRICT_NO_W_DOMAIN))
            {
                return 0;
            }

            return (dname != WIZARD_DOMAIN);
        }

        /* The private directory of a domain is closed. */
        if (wname == "private")
        {
            return 0;
        }

        /* To write something now you need a sanction. */
        return (valid_write_sanction(uid, dname) ||
                valid_write_all_sanction(uid, dname));
        /* Not reached. */

    case "w":
        /* Must be /w/wizard/something at least. */
        if (size < 3)
        {
            return 0;
        }

        /* The wizard must be an existing domain wizard. */
        wname = dirs[1];
        dname = query_wiz_dom(wname);
        if (!sizeof(dname))
        {
            return 0;
        }

        /* A Lord can write anywhere in the domain, and wizards can naturally
         * write their own directory.
         */
        if ((query_domain_lord(dname) == uid) ||
            (wname == uid))
        {
            return 1;
        }

        /* A mentor can write in all the directories of his students. */
        if ((wname in query_students(uid)))
        {
            return 1;
        }

        /* Steward can write anywhere, except in the Lord's directory. */
        if ((query_domain_steward(dname) == uid) &&
            (wname != query_domain_lord(dname)))
        {
            return 1;
        }

        /* The private directory of a wizard is closed to all others. */
        dir = ((size > 3) ? dirs[3] : "");
        if (dir == "private")
        {
            return 0;
        }

        /* To write something now you need a sanction. It must be a personal
         * sanction or a domain 'write all' sanction.
         */
        return (valid_write_sanction(uid, wname) ||
                valid_write_all_sanction(uid, dname));
        /* Not reached. */

    default:
        return 0;
    }
}


/*
 * Function name: valid_read
 * Description  : Checks if a certain user has the right to read a file.
 * Arguments    : string path  - path name of the file to be read.
 *                mixed reader - the object or name of the reader.
 *                string func  - the calling function.
 * Returns      : int 1/0 - allowed/disallowed.
 */
mixed valid_read(string file, string uid, string func, object|lwobject reader)
{
    string *dirs;
    string dname;
    string wname;
    string dir;
    int size;

    /* Everyone is allowed to see the time or size of a file. */
    if ((func == "file_time") ||
        (func == "file_size"))
    {
        return 1;
    }

    /* Allow read in / */
    if (file == "/")
    {
        return 1;
    }

    // logger->debug("Previous: %s", load_name(previous_object()));
    // logger->debug("Current user id: %s", uid);
    /* Root and archwizards and keepers may do as they please. */
    if ((uid == ROOT_UID) || load_name(previous_object()) == LOGIN_OBJECT ||
        (query_wiz_rank(uid) >= WIZ_ARCH))
    {
        // logger->debug("User has root or archwizard privileges.");
        return 1;
    }

    /* Anonymous objects cannot do anything. */
    if (!stringp(uid) ||
        !sizeof(uid))
    {
        return 0;
    }

    dirs = explode(file, "/") - ({ "" });
    size = sizeof(dirs);

    switch(dirs[0])
    {
    /* These directories are closed to all save the admin. */
    case "binaries":
    case "players":
        return 0;
        /* Not reached. */

    case "d":
        /* Allow read in /d/ as such. We need to check for "*" because of ls
         * and "some_file" because of ftp.
         */
        if ((size == 1) ||
            ((size == 2) && ((dirs[1] == "*") || (dirs[1] == "some_file"))))
        {
            return 1;
        }

        dname = dirs[1];
        wname = ((size > 2) ? dirs[2] : "");

        if (dname == BASE_DOMAIN || wname == "doc")
        {
            return 1;
        }

        /* The domain must be an existing domain. */
        if (query_domain_number(dname) == -1)
        {
            return 0;
        }

        /* A Lord or steward can read anywhere in the domain. */
        if ((query_domain_lord(dname) == uid) ||
            (query_domain_steward(dname) == uid))
        {
            return 1;
        }

        /* The open directory of a domain is free for all. */
        if (wname == "open")
        {
            return 1;
        }

        /* The domain dir can be read by all, apart from the private dir
         * which is reserved for the liege/steward
         */
        if (wname == "domain")
        {
            /* The subdir for restrictlogs is open for the mentor who can
             * read the appropriate file
             */
            if ((size > 5) && (dirs[3] == "private") && (dirs[4] == "restrictlog"))
            {
                return (dirs[5] in query_students(uid));
            }
        }

	/* Check for special team directory here, as it is in the base
	 * domain, but not reachable by anyone (i.e. before any sanction
	 * checks are made)
	 */
        dir = ((size > 3) ? dirs[3] : "");
	// if ((dname == BASE_DOMAIN) && (wname == "ateam"))
	// {
	//     /* This can be allowed only if the wizard is a member of the team,
	//      * or if the ateam code is reading in its own dir. Otherwise we
	//      * disallow it.
	//      */
	//     return (dir in query_team_membership(uid)) ||
	// 	((dname == uid) && (sizeof(rpath) > 3) && (rpath[2] == "ateam") &&
	// 	 (rpath[3] == dirs[3]));
	// }

        /* The domain can read in itself, unless it is the domain for lonely
         * wizards.
         */
        if (uid == dname)
        {
            return (dname != WIZARD_DOMAIN);
        }

        /* We have to check for the directory sanctions here because they
         * might disclose the private directories.
         */
        if (sizeof(wname) &&
            recursive_valid_read_path_sanction(uid, dname, dirs[2..]))
        {
            return 1;
        }

        /* The private directory of a wizard is closed to all others. */
        if (dir == "private")
        {
            return 0;
        }

        /* Some people have been granted global read rights. Global read
         * includes the private directory of a domain. Team members all
	 * have global read.
         */
        if (m_global_read[uid] ||
	    sizeof(query_team_membership(uid)) > 0)
	{
            return 1;
	}

        /* Wizards can read everywhere in the domain unless this is the domain
         * for 'lonely' wizards.
         */
        if (query_wiz_dom(uid) == dname)
        {
            /* The exception is restricted wizards, who has to be satisfied
             * with their own, open dirs and specially sanctioned dirs.
             */
            if (query_restrict(uid) & RESTRICT_RW_HOMEDIR)
            {
                return 0;
            }
            return (dname != WIZARD_DOMAIN);
        }

        /* The private directory of a domain is close to all others. */
        if (wname == "private")
        {
            return 0;
        }

        /* To read something now you need a sanction. */
        return (valid_read_sanction(uid, dname) ||
                valid_read_all_sanction(uid, dname));

        /* Not reached. */

    case "w":
        /* Allow read in /w/ as such. We need to check for "*" because of ls
         * and "some_file" because of ftp.
         */
        if ((size == 1) ||
            ((size == 2) && ((dirs[1] == "*") || (dirs[1] == "some_file"))))
        {
            return 1;
        }

        wname = dirs[1];
        /* A wizard can read in his own directory. */
        if (uid == wname)
        {
            return 1;
        }

        dname = query_wiz_dom(wname);
        /* The Lord can always read in the domain of his subject wizards. */
        if (query_domain_lord(dname) == uid)
        {
            return 1;
        }

        /* A mentor can read in all the directories of his students. */
        if ((wname in query_students(uid)))
        {
            return 1;
        }

        /* The open directory of a wizard is free for all. */
        if ((size > 2) && (dirs[2] == "open"))
        {
            return 1;
        }

        /* The private directory of a wizard is closed to all others. */
        if ((size > 2) && (dirs[2] == "private"))
        {
            return 0;
        }

        /* Some people have been granted global read rights. Global read
         * includes the personal directory of a player. Team members
	 * all have global read.
         */
        if (m_global_read[uid] ||
	    sizeof(query_team_membership(uid)) > 0)
        {
            return 1;
        }

        /* To read something now you need a sanction. */
        return (valid_read_sanction(uid, wname) ||
                valid_read_all_sanction(uid, dname));
        /* Not reached. */

    case "syslog":
        /* The syslog directory is open to all, except for the "log"
         * subdirectory.
         */
        if ((size == 1) || (dirs[1] != "log"))
        {
            return 1;
        }

        /* The AoP team and Lords can see various logs. Others cannot see
         * them.
         */
        if ((size > 2) &&
            ((query_wiz_rank(uid) >= WIZ_LORD) ||
              query_team_member("aop", uid)) && (dirs[2] in explode(AOP_TEAM_LOGS, ",")))
        {
            return 1;
        }

        return 0;

    default:
        return 1;
    }
}

#if 0
/*
 * Function name: valid_move
 * Description  : This function is called by the gamedriver to see whether
 *                it is possible to move an object to a destination.
 * Arguments    : object ob   - the object to move.
 *                object dest - the intended destination.
 * Arguments    : int 1/0 - allowed/disallowed.
 */
int
valid_move(object ob, object dest)
{
    return 1;
}

/*
 * Function name: valid_resident
 * Description  : This function is called to see whether the object is allowed
 *                to set the pragma 'resident'. At this point no object is
 *                allowed to do so. That is safest.
 * Arguments    : object ob - the object to test.
 * Returns      : int 1/0 - allowed/disallowed.
 */
int
valid_resident(object ob)
{
    return 0;
}
#endif


/*
 * Function name: valid_debug
 * Description  : This function is called to see whether the object is allowed
 *                to call the efun debug() if the object is anything but this
 *                object SECURITY. Since we don't want other objects to call
 *                debug other than via SECURITY->do_debug(), we disallow it
 *                for all.
 * Arguments    : object ob - the object calling valid_debug
 *                string cmd - the debug command.
 *                mixed arg1 - the argument 1 to debug.
 *                mixed arg2 - the argument 2 to debug.
 *                mixed arg3 - the argument 3 to debug.
 * Returns      : int 1/0 - allowed/ disallowed.
 */
varargs int valid_debug(object ob, string cmd, mixed arg1, mixed arg2, mixed arg3)
{
    return 0;
}


/*
 * Function name: valid_query_ip_ident
 * Description  : This function is called to check whether the object is
 *                allowed to call the efun query_ip_ident() on a particular
 *                target.
 * Arguments    : object actor  - the object that wants to call the efun.
 *                object target - the object the actor wants to know about.
 * Returns      : int 1/0 - allowed/ disallowed.
 */
varargs int valid_query_ip_ident(object actor, object target)
{
    string euid = geteuid(actor);

    /* Only archwizard and keepers may call this efun. */
    return (query_wiz_rank(euid) >= WIZ_ARCH);
}


/*
 * Function name: valid_query_ip
 * Description  : This function is called to check whether the actor is
 *                allowed to call the efun query_ip_number() or _name() on
 *                a particular target.
 * Arguments    : mixed actor   - the actor that wants to call the efun.
 *                object target - the object the actor wants to know about.
 * Returns      : int 1/0 - allowed/ disallowed.
 */
varargs int valid_query_ip(mixed actor, object target)
{
    if (objectp(actor))
    {
        actor = geteuid(actor);
    }

    /* Lords and arches can see ip-number. */
    if (query_wiz_rank(actor) >= WIZ_LORD)
    {
        return 1;
    }

    /* Members of the domain arch and player arch team, too. */
    if (query_team_member("aod", actor) ||
        query_team_member("aop", actor))
    {
        return 1;
    }

    return 0;
}


/*
 * Function name: valid_player_info
 * Description  : Find out whether the actor is allowed to access certain
 *                player information.
 * Arguments    : mixed actor - the actor that wants to know the info.
 *                string name - the name of the person to get info about.
 *                string func - the type of info the actor wants to know.
 * Returns      : int 1/0 - allowed/disallowed.
 */
public int valid_player_info(mixed actor, string name, string func)
{
    if (objectp(actor))
    {
        actor = geteuid(actor);
    }

    /* Root has all access. */
    if (actor == ROOT_UID)
    {
        return 1;
    }

    switch(query_wiz_rank(actor))
    {
    case WIZ_ARCH:
    case WIZ_KEEPER:
	/* Arches and keepers do all. */
	return 1;

    case WIZ_LORD:
	/* Lieges have some special commands. */
	if ((func in ALLOWED_LIEGE_COMMANDS))
	{
	    return 1;
	}
	/* Lords can do onto their subject wizards. */
	if (query_domain_lord(query_wiz_dom(name)) == actor)
	{
	    return 1;
	}
	/* Intentionally no break; */

    default:
	/* Wizard may have been allowed on the commands. */
	if (query_team_member("aod", actor) ||
	    query_team_member("aop", actor))
	{
	    return 1;
	}
    }

    /* All others, no show. */
    return 0;
}


/*
 * Function name: check_snoop_validity
 * Description  : Do the actual validity checking.
 * Arguments    : object snooper - the prospective snooper.
 *                object snopee  - the prospective snoopee.
 *                int sanction - consider sanctioning or not.
 * Returns      : int 1/0 - allowed/disallowed.
 */
static int check_snoop_validity(object snooper, object snoopee, int sanction)
{
    int by_type;
    int on_type;
    string by_name;
    string on_name;

    by_name = geteuid(snooper);
    by_type = query_wiz_rank(by_name);
    on_name = geteuid(snoopee);
    on_type = query_wiz_rank(on_name);

    /* Only wizards can snoop. */
    if (by_type == WIZ_MORTAL)
    {
        return 0;
    }

    /* If the wizard is restricted, he can't snoop anyone */
    if (query_restrict(by_name) & RESTRICT_SNOOP)
    {
        return 0;
    }

    /* Lords and stewards can snoop members everywhere, if the snoopee has a
     * level lower than their own and they can snoop apprentices too.
     */
    if (((by_type == WIZ_LORD) ||
         (by_type == WIZ_STEWARD)) &&
        (((query_wiz_dom(on_name) == query_wiz_dom(by_name)) &&
         (on_type < by_type)) ||
         (on_type == WIZ_APPRENTICE)))
    {
        return 1;
    }

    /* Arch snoops all but arch. */
    if ((by_type >= WIZ_ARCH) &&
        (on_type < WIZ_ARCH))
    {
        return 1;
    }

    /* Mortals are safe in sanctuary for all but arch++. */
    if (environment(snoopee) &&
        (environment(snoopee)->query_prevent_snoop()))
    {
        return 0;
    }

    /* Check for domain restriction */
    if ((SECURITY->query_restrict(by_name) & RESTRICT_SNOOP_DOMAIN) &&
        environment(snoopee)->query_domain() != query_wiz_dom(by_name))
    {
        return 0;
    }

    /* Ordinary wizzes snoops all mortals. */
    if (on_type == WIZ_MORTAL)
    {
        return 1;
    }

    /* A wizard can sanction another wizard to snoop him. */
    if (sanction)
    {
        return valid_snoop_sanction(by_name, on_name);
    }

    /* Mentors can snoop their students. */
    if ((on_name in query_students(snooper->query_real_name())))
    {
        return 1;
    }

    return 0;
}


/* This macro will log the snoop-action when the actor is not a member of
 * the administration.
 */
#ifdef LOG_SNOOP
#define ACTION_SNOOP(str) \
if (query_wiz_rank(caller_name) < WIZ_ARCH) \
{ this_object()->log_syslog(LOG_SNOOP, ctime(time()) + " " + (str)); }
#endif


/*
 * Function name: valid_snoop
 * Description  : Checks if a user has the right to snoop another user.
 * Arguments    : object initiator - the actor for the command.
 *                object snooper   - the prospective snooper.
 *                object snopee    - the prospective snoopee.
 * Returns      : int 1/0 - allowed/disallowed.
 */
public int valid_snoop(object snooper, object snoopee)
{

//     /* Break snoop case. Valid if the breaker has the right to snoop the
//      * person currently doing the snooping and naturally people can break
//      * their own snoop as well. Do not consider sanctioning in this case.
//      */
//     if (!snoopee)
//     {
//         if ((caller_name != actor_name) &&
//             !check_snoop_validity(initiator, snooper, 0))
//         {
//             return 0;
//         }

// #ifdef LOG_SNOOP
//         ACTION_SNOOP(sprintf(" %-11s snoop broken by %s\n",
//                              capitalize(snooper->query_real_name()),
//                              capitalize(caller_name)));
// #endif
//         return 1;
//     }

//     /* Prevent accidental breaking of snoop. If the target is already snooped
//      * it is not possible to set a new snoop on that target.
//      */
//     if (efun::query_snoop(snoopee))
//     {
//         return 0;
//     }

//     /* Set up snoop case. This way player A forces B to snoop C. This is
//      * only valid for archwizards++. We do not consider sanctioning in
//      * this case.
//      */
// //    target_name = geteuid(snoopee);
//     if (caller_name != actor_name)
//     {
//         if ((query_wiz_rank(caller_name) < WIZ_ARCH) ||
//             !check_snoop_validity(initiator, snoopee, 0))
//         {
//             return 0;
//         }

// #ifdef LOG_SNOOP
//         ACTION_SNOOP(sprintf(" %-11s snoops %-11s forced by %s\n",
//                              capitalize(snooper->query_real_name()),
//                              capitalize(snoopee->query_real_name()),
//                              capitalize(caller_name)));
// #endif
//         return 1;
//     }

    /* Ordinary snoop case. A wants to snoop B. Valid if snooper can snoop
     * snoopee. Consider sanctioning in this case.
     */
    if (check_snoop_validity(snooper, snoopee, 1))
    {
// #ifdef LOG_SNOOP
//         ACTION_SNOOP(sprintf(" %-11s snoops %s\n",
//                              capitalize(snooper->query_real_name()),
//                              capitalize(snoopee->query_real_name())));
//         if (query_wiz_rank(caller_name) < WIZ_ARCH)
//         {
//             this_object()->log_syslog(LOG_SNOOP, "    " +
//                 RPATH(object_name(environment(snooper))) + " snoops " +
//                 RPATH(object_name(environment(snoopee))) + "\n");
//         }
// #endif

        return 1;
    }

    return 0;
}


/*
 * Function name: creator_file
 * Description  : Gives the name of the creator of a file. This is a
 *                direct function of the object_name(). The creator must be
 *                a domain or wizard name, the root or backbone euid or
 *                it can be 0 if it is invalid.
 * Arguments    : string str - the path to process.
 * Returns      : string - the name of the creator.
 */
string creator_file(string str)
{
    string *parts;

    /* Get the parts of the name. */
    parts = explode(str, "/") - ({ "" });

    /* The file is probably in a domain directory. */
    if (parts[0] == "d")
    {
        /* The file is owned by an active wizard. */
        if ((sizeof(parts) > 2) &&
            (query_wiz_dom(parts[2]) == parts[1]))
        {
            return parts[2];
        }

        /* The file is owned by an active domain. */
        if (query_domain_number(parts[1]) > -1)
        {
            return parts[1];
        }
    }

    /* The file is from a wizard. */
    if (parts[0] == "w")
    {
        if ((sizeof(parts) > 2) && (sizeof(query_wiz_dom(parts[1]))))
        {
            return parts[1];
        }
    }

    /* Everything in /secure gets return root uid. */
    if (parts[0] == "secure")
    {
        return ROOT_UID;
    }

    /* No cloning or loading from or /open. */
    if (parts[0] == "open")
    {
        return 0;
    }

    /* All else: return backbone uid. */
    return BACKBONE_UID;
}


string modify_path(string path, object ob)
{
    return path;
}


#if 0
string
valid_compile_path(string path, string filename, string fun)
{
    return path;
}
#endif


/*
 * Convert a possibly relative path to an absolute path. We can assume
 * that there is a this_player(). This is called from within the editor.
 */
string make_path_absolute(string path)
{
    // Make sure we have a valid path and player
    if (!stringp(path) || !this_player())
        return 0;
        
    // If it's already an absolute path, return it
    if (path[0] == '/')
        return path;

    // Get the player's current path and name    
    string base_path = this_player()->query_path();
    string wiz_name = this_player()->query_real_name();
    
    // Handle tilde expansion first
    if (path[0] == '~')
        path = TPATH(wiz_name, path);
        
    // Then combine with current path if needed
    return FPATH(base_path, path);
}


/*
 * Function name: load_domain_link
 * Description:   Try to load a domain_link file
 * Arguments:     file - The domain_link to load
 * Returns:       True if everything went ok, false otherwise
 */
static int load_domain_link(string file)
{
    string err, creator;

    if (file[<2..] == ".c") { file = file[..<3]; }
    if (file_size(file + ".c") == -1)
    {
        return 0;
    }

    creator = creator_file(file);
    configure_object(this_object(), OC_EUID, ROOT_UID);

    if (err = ({string})LOAD_ERR(file))
    {
        write("\tCannot load: " + file + ":\n     " + err + "\n");
        return 0;
    }

    write("\t" + file + ".c (" + creator + ")\n");

    catch(file->preload_link());

    return 1;
}


/*
 * Function name: preload_boot
 * Description  : Called at game start time for every file that needs to be
 *                preloaded to load it into memory.
 * Arguments    : string file - the file to preload (without ".c" suffix).
 */
static void preload_boot(string file)
{
    string err, creator;

    if (file_size(file + ".c") == -1)
    {
        return;
    }

    creator = creator_file(file);
    configure_object(this_object(), OC_EUID, ROOT_UID);

    if (err = ({string})LOAD_ERR(file))
    {
        write("\tCan not load: " + file + ":\n     " + err + "\n");
    }
    else
    {
        write("\tPreloading: " + file + ".c  (" + creator + ")\n");
        if (sizeof(file = catch(file->teleledningsanka())))
        {
            write("\tError: " + file + ".c\n");
        }
    }
}


/*
 * Function name: final_boot
 * Description  : This function will be called from the gamedriver when the
 *                game is started, after start_boot() and preload_boot() are
 *                called. Note that this function is not called when the
 *                master is updated.
 */
static void final_boot()
{
    // int theport;

    game_started = 1;
    // theport = debug("mud_port");
    // if (theport)
    // {
    //     set_auth(this_object(), "root:root");
    //     write_file((GAME_START + "." + theport), ctime(time()) + "\n");
    // }

    // debug("set_swap",
    //       ({
    //           SWAP_MEM_MIN,
    //           SWAP_MEM_MAX,
    //           SWAP_TIME_MIN,
    //           SWAP_TIME_MAX
    //           }) );

    /* Tell the graph we rebooted. */
    mark_graph_reboot();

#ifdef UDP_ENABLED
#ifdef UDP_MANAGER
    udp_manager = UDP_MANAGER;
    if (catch(udp_manager->teleledningsanka()))
        udp_manager = 0;

    if (stringp(udp_manager))
        udp_manager->send_startup_udp(MUDLIST_SERVER[0], MUDLIST_SERVER[1]);
    else
#endif
        debug("send_udp", MUDLIST_SERVER[0], MUDLIST_SERVER[1],
              "@@@" + UDP_STARTUP + this_object()->startup_udp() + "@@@\n");
#endif
}


/*
 * Function name: start_shutdown
 * Description  : This function is called by the gamedriver to get a list
 *                of all interactive objects that need to be disconnected
 *                from the game when it is shutting down.
 * Returns      : object * - the interactive users in the game.
 */
object *start_shutdown()
{
    return users();
}


/*
 * Function name: cleanup_shutdown
 * Description  : This function is called for each interactive object when
 *                the game is shutting down. It makes all those players
 *                quit the game.
 * Arguments    : object ob - the object to force to quit.
 */
static void cleanup_shutdown(object ob)
{
    set_this_player(ob);
    ob->quit();
}


/*
 * Function name: final_shutdown
 * Description  : When all mortals are kicked out of the game, this function
 *                is called last by the gamedriver before the game closes.
 */
static void final_shutdown()
{
#ifdef UDP_ENABLED
    if (stringp(udp_manager))
    {
        udp_manager->send_shutdown_udp(MUDLIST_SERVER[0], MUDLIST_SERVER[1]);
        udp_manager->update_masters_list();
    }
#endif

    /* Process the graph data even if this isn't the top of the hour. */
    graph_process_data();

    /* Save the master. */
    save_master();

    /* Call the domain links. */
    ARMAGEDDON->execute_shutdown();
}


/*
 * Function name: log_error
 * Description  : This function is called from the game driver if there is
 *                an error while compiling an object.
 * Arguments    : string path  - the path of the object having the error.
 *                string error - the error message.
 */
// static void log_error(string path, string error)
// {
//     set_auth(this_object(), "root:root");

//     /* Display the message to interactive wizards. */
//     if (this_interactive())
//     {
//         if (this_interactive()->query_wiz_level() ||
//             this_interactive()->query_prop(PLAYER_I_SEE_ERRORS))
//         {
//             this_interactive()->catch_tell(error);
//         }
//     }

//     /* Create the log directory if necessary. */
//     path = query_wiz_path(creator_file(path)) + "/log";
//     if (file_size(path) != -2)
//     {
//         mkdir(path);
//     }
//     path += "/errors";
//     write_file(path, error);
// }


/*
 * This function is called from GD when rooms are destructed so that master
 * can move players to safety.
 */
void destruct_environment_of(object ob)
{
    if (environment(ob))
    {
        catch(ob->move(environment(ob)));
    }

    if (!interactive(ob))
    {
        return;
    }
    ob->move_living("X", ob->query_default_start_location());
}


/*
 * Function name: define_include_dirs
 * Description  : Define  where the '#include' statement is supposed to
 *                search for files. "." will automatically be searched
 *                first, followed in order as given below. The path should
 *                contain a '%s', which will be replaced by the file
 *                searched for.
 * Returns      : string * - the array of path to search.
 */
string *define_include_dirs()
{
    return ({ "/sys/%s" });
}


/*
 * Function name: get_ed_buffer_save_object_name
 * Description  : When a wizard is in ed and goes linkdead, this function
 *                is called to get the name of the save-file. If the
 *                wizard does not have the necessary rights, the function
 *                will disallow save.
 * Arguments    : string - the name of the file to save
 * Returns      : string - the name of the secured save-file
 *                0      - the player has no write-rights.
 */
string get_ed_buffer_save_object_name(string file)
{
    string *path;

    if (!objectp(this_player()))
        return 0;

    if (!valid_write(file, getuid(this_player()), "ED", this_player()))
        return 0;

    path = explode(file, "/");
    path[sizeof(path) - 1] = "dead_ed_" + path[sizeof(path) - 1];

    return implode(path, "/");
}


/* save_ed_setup and restore_ed_setup are called by the ed to maintain
   individual options settings. These functions are located in the master
   object so that the local gods can decide what strategy they want to use.

/*
 * The wizard object 'who' wants to save his ed setup. It is saved in the
 * file ~wiz_name/.edrc . A test should be added to make sure it is
 * a call from a wizard.
 *
 * Don't care to prevent unauthorized access of this file. Only make sure
 * that a number is given as argument.
 */
int save_ed_setup(object who, int code)
{
    string file;

    if (!intp(code))
        return 0;
    file = query_wiz_path(({string})who->query_real_name()) + "/.edrc";
    rm(file);
    return write_file(file, code + "");
}


/*
 * Retrieve the ed setup. No meaning to defend this file read from
 * unauthorized access.
 */
int retrieve_ed_setup(object who)
{
    string file;
    int code;

    file = query_wiz_path(who->query_real_name()) + "/.edrc";
    if (file_size(file) <= 0)
        return 0;
    sscanf(read_file(file), "%d", code);
    return code;
}


/*
 * Function name: query_allow_shadow
 * Description  : This function is called from the game driver to find out
 *                whether it is allowed to shadow a particular object. The
 *                object that wants to shadow is previous_object(). To
 *                prevent shadowing, the target object will have to define
 *                the function query_prevent_shadow() to return 1.
 * Arguments    : object target - the object targeted for shadowing.
 * Returns      : int 1/0 - allowed/disallowed.
 */
int query_allow_shadow(object target)
{
    return !(target->query_prevent_shadow(previous_object()));
}


/*
 * Function name: valid_exec
 * Description:   Checks if a certain 'program' has the right to use exec()
 * Arguments:     name: Name of the 'program' that attempts to use exec()
 *                      Note that this is different from object_name(),
 *                      The program name is what calling_program returns.
 *                to:   destination of socket
 *                from: target of the socket
 * Returns:       True if exec() is allowed.
 */
int valid_exec(string name, object to, object from)
{
    name = "/" + name;
    if ((name == (LOGIN_OBJECT + ".c")) ||
        (name == (POSSESSION_OBJECT + ".c")) ||
        (name == (LOGIN_TEST_PLAYER + ".c")) ||
        (name == (LOGIN_NEW_PLAYER + ".c")))
    {
        return 1;
    }

    return 0;
}


/*
 * Function name: simul_efun_reload
 * Description  : This function sets the authorisation variables for the
 *                simul_efun object.
 */
void simul_efun_reload()
{
    configure_object(find_object(SIMUL_EFUN), OC_EUID, ROOT_UID);
}


/*
 * Function name: loaded_object
 * Description  : This function is called when an object is loaded into
 *                memory by another object. It tests whether it was valid
 *                to load the object and sets the authorisation variables
 *                in the loaded object. If the load was not valid, throw()
 *                will terminate the execution.
 * Arguments    : object lob - the loading object.
 *                object ob  - the loaded object.
 */
void loaded_object(object lob, object ob)
{
    string creator = creator_object(ob);
    string *auth = ({ geteuid(lob), getuid(lob) });

    if (!sizeof(creator))
    {
        do_debug("destroy", ob);
        throw("Loading a bad object from: " + object_name(lob) + ".\n");
    }

    if (auth[1] == "0")
    {
        creator = object_name(ob);
        do_debug("destroy", ob);
        throw("Unauthorized load: " + creator + " by: " + object_name(lob) + ".\n");
    }

    if ((creator == BACKBONE_UID) ||
        (creator == auth[0]))
    {
        configure_object(ob, OC_EUID, auth[1]);
        return;
    }

    configure_object(ob, OC_EUID, creator);
}


/*
 * Function name: cloned_object
 * Description  : This function is called when an object is cloned. It
 *                tests whether the clone was valid. It also sets the
 *                authorisation variable in the cloned object. If the
 *                clone was not valid, throw() will terminate the
 *                execution.
 * Arguments    : object cob - the cloning object.
 *                object ob  - the cloned object.
 */
void cloned_object(object cob, object ob)
{
    string creator = creator_object(ob);
    string *auth = ({ geteuid(cob), getuid(cob) });

    if (!sizeof(creator))
    {
        creator = object_name(ob);
        do_debug("destroy", ob);
        throw("Unauthorized clone: " + creator + " by: " +
            object_name(cob) + ".\n");
    }

    if (auth[1] == "0")
    {
        creator = object_name(ob);
        do_debug("destroy", ob);
        throw("Cloning without privilege: " + creator + " by: " +
            object_name(cob) + ".\n");
    }

    if ((creator == BACKBONE_UID) ||
        (creator == auth[0]))
    {
        configure_object(ob, OC_EUID, auth[1]);
        return;
    }

    configure_object(ob, OC_EUID, creator);
}


/*
 * Function name: modify_command
 * Description  : Modify a command given by a certain living object. This can
 *                be used for many quicktyper-like functions. There are also
 *                some master.c defined substitutions. Commands that start
 *                with a dollar ($) are not substituted.
 * Arguments    : string cmd - the command to modify.
 *                object ob - the object for which to modify the command.
 * Returns      : string - the modified command to execute.
 */
string modify_command(string cmd, object ob)
{
    string domain;
    int no_subst;

    if (!sizeof(cmd))
    {
        return cmd;
    }

    while(cmd[0] == '$')
    {
        cmd = cmd[1..];
        no_subst = 1;
    }

    while(cmd[0] == ' ')
    {
        cmd = cmd[1..];
    }



    if (cmd in command_substitute)
    {
        if (interactive(ob) && !ob->query_wiz_level() &&
            pointerp(m_domains[domain = environment(ob)->query_domain()]))
        {
            m_domains[domain][FOB_DOM_CMNDS]++;
        }
        return command_substitute[cmd];
    }

    /* No modification for NPC's */
    if (!interactive(ob))
    {
        return cmd;
    }

    /* Count commands for ranking list */
    if (environment(ob) &&
        !ob->query_wiz_level() &&
        pointerp(m_domains[domain = environment(ob)->query_domain()]))
    {
        m_domains[domain][FOB_DOM_CMNDS]++;
    }

    /* Allow modification if it does not start with a "$". */
    if (!no_subst)
    {
        cmd = ({string}) ob->modify_command(cmd);
    }

    /* We can not allow any handwritten VBFC */
    while(strstr(cmd, "@@") != -1)
    {
        cmd = implode(explode(cmd, "@@"), "#");
    }

    return cmd;
}


/*
 * Function name: query_move_opposites
 * Description  : Returns the exact pointer to the mapping move_opposites.
 * Arguments    : mapping - the mapping.
 */
mapping query_move_opposites()
{
    logger->debug("Querying move opposites.");
    
    /* We intentionally return the unmodified mapping! */
    return move_opposites;
}


/*
 * Function name: query_memory_percentage
 * Description  : This function will return the percentage of memory usage
 *                of the game so far. When the counter reaches 100, it is
 *                time to reboot.
 * Returns      : int - the relative memory usage.
 */
nomask public int query_memory_percentage()
{
    string foobar, *data = explode(SECURITY->do_debug("malloc"), "\n");
    int    f, cval, sz = sizeof(data);

    // This code relies heaviily on the ssbrk line being last. Good enough.
    if (sscanf(data[sz-1],
	       "%ssbrk requests: %d %d (a) %s", foobar, f, cval, foobar) == 4)
	return (cval / (memory_limit / 100));
    else
	return 0;
}


/*
 * Function name: memory_failure
 * Description:   This function is called when the gamedriver considers
 *                itself in trouble and need the game shut down in a graceful
 *                manner. This function _must_ be called via a call_other. It
 *                may only be called by root itself or by a member of the
 *                administration.
 */
static void memory_failure()
{
    if (!mem_fail_flag)
    {
        mem_fail_flag = 1;

        configure_object(this_object(), OC_EUID, ROOT_UID);
        ARMAGEDDON->start_shutdown("The memory is almost used up!", 10,
            ROOT_UID);
    }
}


/*
 * Function name: memory_reconfigure
 * Description  : This function is called when the gamedriver receives
 *                an external signal, denoting that the memory status
 *                has changed.
 * Arguments    : int mem - Memory size, 0 small, 1 large.
 */
static void memory_reconfigure(int mem)
{
    string mess = "a different";
    object *list;

#ifdef LARGE_MEMORY_LIMIT
    if (((mem == 0) &&
         (memory_limit == SMALL_MEMORY_LIMIT)) ||
        ((mem == 1) &&
         (memory_limit == LARGE_MEMORY_LIMIT)))
    {
        return;
    }

    if (mem == 0)
    {
        memory_limit = SMALL_MEMORY_LIMIT;
        mess = "small";
    }
    else
    {
        memory_limit = LARGE_MEMORY_LIMIT;
        mess = "large";
    }
    check_memory(0);
#endif

    list = filter(users(), (: $1->query_wiz_level() :));
    list->catch_tell("@ Armageddon: I have switched to " + mess + " memory mode.\n");
}


/*
 * This function is called if the driver gets sent a signal that it catches.
 */
static void external_signal(string sig_name)
{
    write("Received " + sig_name + " signal.\n");
    switch (sig_name)
    {
    case "INT":
        ARMAGEDDON->start_shutdown("Someone halted the game!", 0, ROOT_UID);
        break;
    case "HUP":
    case "KILL":
    case "QUIT":
    case "TERM":
        shutdown();
        break;
    case "USR1":
        memory_reconfigure(0);
        break;
    case "USR2":
        memory_reconfigure(1);
        break;
    case "TSTP":
    case "CONT":
        break;
    case "UNKNOWN":
    default:
        write("Unknown signal \"" + sig_name + "\"received!\n");
    }
}


/*
 * Function name: query_memory_limit
 * Description:   This function returns the current memory limit.
 */
public int query_memory_limit()
{
    return memory_limit;
}


/*
 * Function name: query_memory_failure
 * Description:   This function returns 1 if memory failure is detected.
 */
public int query_memory_failure()
{
    return mem_fail_flag;
}


#if 0
/*
 * Function name: valid_save_binary
 * Description  : This function is called when a file has ordered the GD
 *                to save a binary image of the program. This might not
 *                be allowed by any and every file so master is asked.
 * Arguments    : string filename - the filename of the object.
 */
int
valid_save_binary(string filename)
{
    return 1;
}


/*
 * Function name: valid_inherit
 * Description  : This function is called when a file has asked to inherit
 *                object. This might not be allowed by any and every file so
 *                master is asked.
 * Arguments    : object ob - the object to inherit withing to inherit.
 *                string inherit_filename - the filename to be inherited.
 */
int
valid_inherit(object ob, string inherit_file)
{
    return 1;
}


/* ob trying to load file */
int
valid_load(object ob, string file)
{
    return 1;
}
#endif


/*
 * Function name:   master_reload
 * Description:     Called from GD after a reload of the master object
 */
void master_reload()
{
}


void recreate(object old_master)
{
    create();
    game_started = 0;
    start_boot(1); /* This does what we want */
    game_started = 1;
#ifdef UDP_ENABLED
#ifdef UDP_MANAGER
    udp_manager = UDP_MANAGER;
#endif
#endif
}


/*
 * Function name: incoming_udp
 * Description:   Called from GD if a udp message has been received. This
 *                can only happen if CATCH_UDP_PORT has been defined in
 *                the GD's config.h file.
 * Arguments:     from_host: The IP number of the sending host
 *                message:   The message sent.
 */
void incoming_udp(string from_host, string message)
{
#ifdef UDP_ENABLED
    if (stringp(udp_manager))
    {
        udp_manager->incoming_udp(from_host, message);
    }
#ifdef LOG_LOST_UDP
    else
    {
        set_auth(this_object(), "#:root");
        log_file(LOG_LOST_UDP, "(" + from_host + ") " + message + "\n", -1);
    }
#endif
#endif
}


static void mark_quit(object player)
{
    string text;
    int index = 0;
    object prev;

    if ((player->query_linkdead()) ||
        (!player->query_wiz_level() &&
         (interactive(player)) &&
         (interactive_info(player, II_IDLE) > MAX_IDLE_TIME)) ||
        ((player == this_interactive()) &&
         ((query_verb() == "quit") ||
          !sizeof(query_verb()))))
    {
        return;
    }

    text = "Current time: " + ctime(time()) + "\nDestructed  : " +
        capitalize(player->query_real_name()) + ".\n";
    if (objectp(this_interactive()))
    {
        text += "Interactive : " +
            capitalize(this_interactive()->query_real_name()) + ".\n";
    }
    if (sizeof(query_verb()))
    {
        text += "Queried verb: " + query_verb() + "\n";
    }

    while(objectp(prev = previous_object(index)))
    {
        text += "    call: " + object_name(prev) + "  " +
            driver_info(DI_TRACE_CURRENT)[index][TRACE_NAME] + "()";
        if (interactive(prev))
        {
            text += "  [" + capitalize(prev->query_real_name()) + "]";
        }
        text += "\n";

        if ((MASTER_OB(prev) + ".c") != ("/" + driver_info(DI_TRACE_CURRENT)[index][TRACE_PROGRAM]))
        {
            text += "    file: /" + driver_info(DI_TRACE_CURRENT)[index][TRACE_PROGRAM] + "\n";
        }

        index--;
    }

    configure_object(this_object(), OC_EUID, ROOT_UID);
    write_file("/syslog/log/DESTRUCTED", text + "\n");
}


/*
 * Function name: remove_interactive
 * Description  : Called from GD if a player logs out or goes linkdead. If
 *                the player quit the game, we don't do anything.
 * Arguments    : object ob    - the player that leaves the game.
 *                int linkdead - true if the player linkdied.
 */
static void remove_interactive(object ob, int linkdied)
{
    string master_ob;

    QUEUE->dequeue(ob);

    /* If someone who is logging in linkdies, we just dispose of it. Also,
     * people who are trying to create a character, will have to start
     * over again.
     */
    master_ob = MASTER_OB(ob);
    if ((master_ob == LOGIN_OBJECT) ||
        (master_ob == LOGIN_NEW_PLAYER) ||
        (master_ob == LOGIN_TEST_PLAYER) ||
        (master_ob == GAMEINFO_OBJECT))
    {
        ob->remove_object();
        return;
    }

    /* Player left the game. */
    if (!linkdied)
    {
        /* Notify the wizards of the fact that the player quit. */
        notify(ob, 1);
        mark_quit(ob);
        return;
    }

    /* Notify the wizards of the linkdeath. */
    notify(ob, 2);

#ifdef STATUE_WHEN_LINKDEAD
    ob->linkdie();
#endif
}


/*
 * Function name: gamedriver_message
 * Description  : This function may (only) be called by the gamedriver to
 *                give a message to all players if that is necessary.
 * Arguments    : string str - the message to tell the people
 */
static void gamedriver_message(string str)
{
    users()->catch_tell(str);
}


/*
 * Function name: runtime_error
 * Description  : In case a runtime error occurs, we tell the message to
 *                the people who need to hear it.
 * Arguments    : string error   - the error message.
 *                object ob      - the object that has the error.
 *                string program - the program name of the error.
 *                string file    - the filename of the error.
 */
static void runtime_e_error(string error, object ob, string prog, string file)
{
    string fmt_error;
    string path = "";
    string mortal = "";

    /* Display the message to interactive wizards. */
    if (this_interactive())
    {
        if (this_interactive()->query_wiz_level() ||
            this_interactive()->query_prop(PLAYER_I_SEE_ERRORS))
        {
            fmt_error =
                "\n\nRuntime error: " + error +
                "       Object: " + (ob ? object_name(ob) : "<???>") +
                "\n      Program: " + prog +
                "\n         File: " + file + "\n\n";
            this_interactive()->catch_tell(fmt_error);
            return;
        }

        /* Tell the player an error occured, but not which error. */
        this_interactive()->catch_tell("Your sensitive mind notices " +
            "a wrongness in the fabric of space.\n");
        mortal = capitalize(this_interactive()->query_real_name());
    }

    configure_object(this_object(), OC_EUID, ROOT_UID);

    /* Create the log directory if necessary. */
    if (objectp(ob))
    {
        path = query_wiz_path(creator_object(ob));
    }
    path += "/log";
    if (file_size(path) != -2)
    {
        mkdir(path);
    }
    path += "/runtime";

    fmt_error = ctime(time()) +
        (sizeof(mortal) ? " (Interactive mortal: " + mortal + ")" : "") + "\n" +
        "Runtime error: " + error +
        "       Object: " + (ob ? object_name(ob) : "<???>") + "\n" +
        "      Program: " + prog + "\n" +
        "         File: " + file + "\n";

    write_file(path, fmt_error + "\n");
}


/*
 *    ----------------------------------------------------------------
 *    The code below this divisor is never called from the gamedriver.
 *    ----------------------------------------------------------------
 */

/*
 * Function name: remove_binary
 * Description  : This function removes a binary file from the game. This
 *                is (presently) necessary when updating a file because
 *                otherwise the binary isn't updated when you move an older
 *                file over it.
 * Arguments    : string path - the fully qualified path to the file,
 *                     excluding the "/binaries" part.
 * Returns      : int 1/0 - success/failure.
 */
public int remove_binary(string path)
{
    /* Add the .c suffix if necessary. */
    if (!(path[..<2] == ".c"))
    {
        path = path + ".c";
    }

    /* We don't have to check for .. constructs here, since the gamedriver
     * will not allow those.
     */
    return rm("/binaries" + path);
}


/*
 * Function name: remove_playerfile
 * Description:   This function moves a playerfile from /players/<?>/
 *                to /players/deleted/<?>/
 *                It also adds some text to the log DELETED.
 * Arguments:     string player - The player to remove
 *                string reason - The reason to why the file is removed
 * Returns:       True if everything went ok, false other wise
 */
public int remove_playerfile(string player, string reason)
{
    string file;
    string deleted;
    int number = 1;

    /* May only be called from the Arch soul. */
    if (!CALL_BY(WIZ_CMD_ARCH))
    {
        return 0;
    }

    file = PLAYER_FILE(player) + ".o";
    deleted = DELETED_FILE(player) + ".o";

    /* Make sure the directories exist for the new player name */
    string dir = "/players";
    if(file_size(dir) == -1)
    {
        mkdir(dir);
    }
    dir += "/deleted";
    if(file_size(dir) == -1)
    {
        mkdir(dir);
    }

    /* If there is a file, move it to the deleted dir. */
    if (file_size(file) < 0)
    {
        return 0;
    }

    if (file_size(deleted) != -1)
    {
        while (file_size(deleted + "." + number) != -1)
              number++;

        deleted += "." + number;
    }

    /* Move the file */
    if (!rename(file, deleted))
    {
        return 0;
    }

    /* Inform the domains of the deletion. */
    map(query_domain_links(), #'find_object)->domain_delete_player(player);

    /* Log the action */
    log_file("DELETED", ctime(time()) + " " + capitalize(player) +
        " by " + capitalize(this_interactive()->query_real_name()) +
        " (" + reason + ")\n", -1);

    return 1;
}


/*
 * Function name: rename_playerfile
 * Description  : This renames a player.
 * Arguments    : string oldname - the old name of the player.
 *                string newname - the new name of the player.
 * Returns      : int 1/0 - success/failure.
 */
public int rename_playerfile(string oldname, string newname)
{
    mapping playerfile;

    /* May only be called from the Arch soul. */
    if (!CALL_BY(WIZ_CMD_ARCH))
    {
        return 0;
    }

    /* Rename the playerfile itself. */
    if (!rename(PLAYER_FILE(oldname) + ".o",
        PLAYER_FILE(newname) + ".o"))
    {
        notify_fail("Renaming playerfile failed!\n");
        return 0;
    }

    /* The name should also be updated in the playerfile. */
    playerfile = restore_value(read_file(PLAYER_FILE(newname)));
    playerfile["name"] = newname;

    /* Make sure the directories exist for the new player name */
    string dir = "/players";
    if(file_size(dir) == -1) { mkdir(dir); }

    write_file(PLAYER_FILE(newname), save_value(playerfile));
    write("Player " + capitalize(oldname) + " successfully renamed to " +
        capitalize(newname) + ".\n");

    /* Rename the mail folder if there is one. */
    if (file_size(FILE_NAME_MAIL(oldname) + ".o") > 0)
    {
        if (rename(FILE_NAME_MAIL(oldname) + ".o",
            FILE_NAME_MAIL(newname) + ".o"))
        {
            write("Mail folder found and renamed.\n");
        }
        else
        {
            write("Mail folder found, but renaming failed.\n");
        }
    }

    /* Update a wizard. */
    if (query_wiz_rank(oldname))
    {
        rename_wizard(oldname, newname);
    }

    log_file("DELETED", ctime(time()) + " " + capitalize(oldname) +
        " -> " + capitalize(newname) + ", renamed by " +
        capitalize(this_interactive()->query_real_name()) + ".\n", -1);
    return 1;
}


/*
 * Function name: proper_password
 * Description  : This function can be used to check whether a certain
 *                password is less likely to be broken using a general
 *                cracker. Therefore the following is checked:
 *                - the password must at least be 6 characters long;
 *                - the password must contain at least one 'non-letter';
 *                - this letter may not be the first or the last letter;
 * Arguments    : string str - the password to check.
 * Returns      : int 1/0 - proper/bad.
 */
int proper_password(string str)
{
    int index = -1;
    int size;
    int normal;
    int stage = 0;

    /* Length of the password must be at least 6 characters. */
    size = sizeof(str);
    if (size < 6)
    {
        return 0;
    }

    /* This may seem a little strange, but it actually is quite simple. As
     * stated before, there should at least be one non-letter and that
     * should be between normal letters. Therefore, starting at stage 0, we
     * wait until we find a letter. Have we found that, more to
     * stage 1 and wait for a non-letter. Then, at stage 2 we again wait for
     * a normal letter. If all checks out, we should be at stage 3 at the
     * end.
     */
    str = lower_case(str);
    while(++index < size)
    {
        normal = ((str[index] >= 'a') && (str[index] <= 'z'));

        switch(stage)
        {
        case 0:
            stage = (normal ? 1 : 0);
            break;

        case 1:
            stage = (normal ? 1 : 2);
            break;

        case 2:
            stage = (normal ? 3 : 2);
            break;
        }
    }

    /* If the stage isn't 3, the password isn't good. */
    return (stage == 3);
}


/*
 * Function name: generate_password
 * Description  : This function will generate a six character password that
 *                consists of letters, numbers and or special characters.
 *                Passwords generated with this function are considered to
 *                be safe enough other than for a brute force attack.
 * Returns      : string - the password.
 */
public string generate_password()
{
    string tmp = "";
    int    size = 8;
    int    index;

    while(--size >= 0)
    {
        switch(random(5))
        {
        case 0:
            /* With 20% change, add a single digit */
            tmp += ("" + random(10));
            break;

        case 1:
            /* With 20% chance, add a "special" character */
            index = random(24);
            tmp += "!@?$%^&*()[]{};:<>,.-_=+"[index..index];
            break;

        case 2..4:
        default:
             /* With 60% chance, add a letter, capitalized with 50% chance */
            index = random(26);
            tmp += (random(2) ? ALPHABET[index..index] :
                capitalize(ALPHABET[index..index]));
            break;
        }
    }

    return tmp;
}


/*
 * Function name: remote_setuid
 * Description  : With this function the COMMAND_DRIVER can request that
 *                its authorisation information is reset in order to allow
 *                for a new uid/euid when another player uses the same soul.
 */
void remote_setuid()
{
    if (function_exists("open_soul", previous_object()) == COMMAND_DRIVER)
    {
        configure_object(previous_object(), OC_EUID, BACKBONE_UID);
    }
}


/*
 * Function name: set_helper_soul_euid
 * Description  : The helper soul is intended for wizards to get some commands
 *                for abilities they would not normally enjoy. Thus, the soul
 *                requires a higher access level than normally. Make sure that
 *                only the right object qualifies, though.
 */
public void set_helper_soul_euid()
{
    if (object_name(previous_object()) == WIZ_CMD_HELPER)
    {
        configure_object(previous_object(), OC_EUID, ROOT_UID);
    }
}


/*
 * Function name: creator_object
 * Description  : Gives the name of the creator of an object. This is a
 *                direct function of the object_name() of the object.
 * Arguments    : object obj - the object to get the creator from.
 * Returns      : string - the name of the domain or wizard who created
 *                         the object.
 */
string creator_object(object obj)
{
    if (!objectp(obj))
    {
        return 0;
    }

    return creator_file(object_name(obj));
}


/*
 * Function name: domain_object
 * Description:   Gives the name of the domain of an object. This is a
 *                direct function of the object_name() of the object.
 * Arguments    : object obj - the object to test.
 * Returns      : string - the name of the domain.
 */
string domain_object(object obj)
{
    string str;
    string dom;
    string wiz;
    string name;

    if (!obj)
    {
        return 0;
    }

    str = object_name(obj);
    if (str[0..2] != "/d/")
    {
        return 0;
    }

    sscanf(str, "/d/%s/%s/%s", dom, wiz, name);
    return dom;
}


/*
 * Function name: load_player
 * Descripton:    This function is called from /std/player_sec
 *                  when the player object is loaded initially.
 *                It sets the euid of the player to root for
 *                the duration of the load.
 */
int load_player()
{
    int res;
    object pobj;

    pobj = previous_object();

    if (function_exists("load_player", pobj) != PLAYER_SEC_OBJECT ||
        !LOGIN_NEW_PLAYER->legal_player(pobj))
    {
        logger->error("Load player not allowed");
        return 0;
    }
    else
    {
        configure_object(pobj, OC_EUID, ROOT_UID);
        res = ({int}) pobj->load_player(pobj->query_real_name());
        configure_object(pobj, OC_EUID, to_string(pobj));
        return res;
    }
}


/*
 * Function name: save_player
 * Description:   Saves a player object.
 */
int save_player()
{
    int res;
    object pobj;

    pobj = previous_object();

    logger->debug("Saving player: %O", pobj);

    if ((function_exists("save_player", pobj) != PLAYER_SEC_OBJECT) ||
        !LOGIN_NEW_PLAYER->legal_player(pobj))
    {
        return 0;
    }
    else
    {
        pobj->fix_saveprop_list();
        res = ({int})pobj->save_player(pobj->query_real_name());
        pobj->open_player();
        return res;
    }
}


/*
 * Function name: store_predeath
 * Description  : Just before the death-modifications are made to a player
 *                file, we secure a copy. Note: this routine moves the
 *                player file. It relies on a new copy being saved shortly
 *                after the call to this routine, so we don't have to make
 *                a costly copy, but can rely on moving it.
 */
void store_predeath()
{
    object pobj;
    string pfile, prefile;

    pobj = previous_object();
    if (function_exists("load_player", pobj) != PLAYER_SEC_OBJECT)
        return;

    pfile = PLAYER_FILE(pobj->query_real_name());
    prefile = pfile + ".predeath.o";
    pfile += ".o";
    /* Rename the present file. A new file will be made shortly. */
    if (get_dir(pfile, GETDIR_DATES)[0] > 0)
        rename(pfile, prefile);
}


int rem_def_start_loc(string str)
{
    if (!def_locations)
        def_locations = STARTING_PLACES;

    if (query_wiz_rank(this_interactive()->query_real_name()) < WIZ_ARCH)
    {
        write("Only arches or keepers may remove starting locations.\n");
        return 1;
    }

    def_locations -= ({ str });
    def_locations = sort_array(def_locations, #'>);
    save_master();

    return 0;
}


int add_def_start_loc(string str)
{
    if (!def_locations)
    {
        def_locations = STARTING_PLACES;
    }

    if (query_wiz_rank(this_interactive()->query_real_name()) < WIZ_ARCH)
    {
        write("Only arches or keepers may add starting locations.\n");
        return 1;
    }

    if (file_size(str + ".c") < 0)
    {
        write("No such file: " + str + "\n");
        return 1;
    }

    /* Delete copies */
    def_locations -= ({ str });
    def_locations = sort_array(def_locations + ({ str }), #'>);
    save_master();
    return 0;
}


int rem_temp_start_loc(string str)
{
    if (!temp_locations)
    {
        temp_locations = TEMP_STARTING_PLACES;
    }

    if (query_wiz_rank(this_interactive()->query_real_name()) < WIZ_ARCH)
    {
        write("Only arches or keepers may remove starting locations.\n");
        return 1;
    }

    /* Delete copies */
    temp_locations -= ({ str });
    temp_locations = sort_array(temp_locations, #'>);
    save_master();
    return 0;
}


int add_temp_start_loc(string str)
{
    if (query_wiz_rank(this_interactive()->query_real_name()) < WIZ_ARCH)
    {
        write("Only arches or keepers may add starting locations.\n");
        return 1;
    }

    if (file_size(str + ".c") <= 0)
    {
        write("No such file: " + str + "\n");
        return 1;
    }
    if (!sizeof(temp_locations))
    {
        temp_locations = TEMP_STARTING_PLACES;
    }

    /* Delete copies */
    temp_locations -= ({ str });
    temp_locations = sort_array(temp_locations + ({ str }), #'> );
    save_master();
    return 0;
}


public string *query_list_def_start()
{
    return def_locations;
}


public string *query_list_temp_start()
{
    return pointerp(temp_locations) ? temp_locations : ({});
}


int check_temp_start_loc(string str)
{
    if (pointerp(temp_locations))
        return member(temp_locations, str);
    return 0;
}


int check_def_start_loc(string str)
{
    if (!def_locations)
        def_locations = STARTING_PLACES;
    return (str in def_locations);
}


public varargs void log_syslog(string file, string text, int length = 0)
{
    string fname;

    fname = driver_info(DI_TRACE_CURRENT)[0][TRACE_PROGRAM];

    if ((fname[0..6] != "secure/") &&
        (fname[0..3] != "cmd/") &&
        (fname[0..3] != "std/"))
    {
        return;
    }

    log_file(file, text, length);
}


void log_public(string file, string text)
{
    int fsize, msize;
    string fname;

    fname = driver_info(DI_TRACE_CURRENT)[0][TRACE_PROGRAM];

    file = OPEN_LOG_DIR + "/" + file;

    if ((fname[0..5] != "secure") &&
        (fname[0..2] != "std"))
    {
        return;
    }

#ifdef CYCLIC_LOG_SIZE
    fsize = file_size(file);
    msize = CYCLIC_LOG_SIZE["root"];

    if (msize > 0 && fsize > msize)
        rename(file, file + ".old");
#endif /* CYCLIC_LOG_SIZE */

    write_file(file, text);
}


void log_restrict(string verb, string arg)
{
    string path,
           dom = query_wiz_dom(getuid(previous_object())),
           wiz = previous_object()->query_real_name();

    if (dom == "")
        dom = BASE_DOMAIN;

    path = "/d";
    if(file_size(path) == -1) { mkdir(path); }
    path += "/" + dom;
    if(file_size(path) == -1) { mkdir(path); }
    path += "/domain";
    if (file_size(path) == -1) { mkdir(path); }
    path += "/private";
    if (file_size(path) == -1) { mkdir(path); }
    path += "/restrictlog";
    if (file_size(path) == -1) { mkdir(path); }

    if (!stringp(verb))
        verb = "";

    // Restrict the size to 50 kb
    if (file_size(path + "/" + wiz) > 51200)
        rename((path + "/" + wiz), (path + "/" + wiz + ".old"));

    write_file(path + "/" + wiz, ctime(time()) + ": " + verb + " " + arg + "\n");
}


/*
 * Function name: query_player_file_time()
 * Description  : This function will return the file-time the playerfile
 *                of a player was last saved.
 *                This routine in its current incarnation is obsolete!
 *                The logout_time is now included in the player file.
 * Arguments    : string pl_name - the name of the player.
 * Returns      : int - the file-time of the players save-file.
 */
int query_player_file_time(string pl_name)
{
    if (!sizeof(pl_name))
    {
        return 0;
    }

    pl_name = lower_case(pl_name);
    configure_object(this_object(), OC_EUID, ROOT_UID);
    return get_dir(PLAYER_FILE(pl_name) + ".o", GETDIR_DATES)[0] || 0;
}


/*
 * Function name: exist_player
 * Description  : Checks whether a player exist or not.
 * Arguments    : string pl_name: the name of the player to check.
 * Returns      : int 1/0 - true if the player exists.
 */
int exist_player(string pl_name)
{
    if (!sizeof(pl_name))
    {
        return 0;
    }

    pl_name = lower_case(pl_name);
    return (file_size(PLAYER_FILE(pl_name) + ".o") > 0);
}


/*
 * Function name: finger_player
 * Description:   Returns a player object restored with the values from
 *                the players save file.
 * Arguments:     pl_name: Name of player
 *                file: (optional) Name of player_file
 * Returns:       The restored player object or 0 if it doesn't exist.
 */
varargs object finger_player(string pl_name, string file)
{
    object ob;
    int ret;
    string f;

    if (!exist_player(pl_name))
    {
        return 0;
    }

    if (!file)
    {
        file = FINGER_PLAYER;
    }

    ob = clone_object(file);

    f = function_exists("load_player", ob);
    if (f != PLAYER_SEC_OBJECT && f != FINGER_PLAYER)
    {
        do_debug("destroy", ob);
        return 0;
    }

    ob->master_set_name(pl_name);
    ob->open_player();               /* sets euid == 0 */
    ret = ob->load_player(pl_name);

    ob->set_trusted(1);
    ob->set_adj(0);                        /* Set the adjectives correctly */

    if (ret)
        return ob;
    else
    {
        ob->remove_object();
        return 0;
    }
}


/*
 * Function name: note_something
 * Description  : This function is called from the info.c commandsoul when
 *                someone made a report. It distuishes between sys-reports
 *                and room-related reports and writes them to the correct
 *                directory.
 * Arguments:     string str - the message to log.
 *                int id     - the id (type) of the log.
 *                object env - the environment of this_player().
 */
void note_something(string str, int id, object env)
{
    string file;
    string ts = ctime(time());
    string text = ts[4..10] + ts[-4..] + ts[10..15]; /* mmm dd yyyy HH:MM */

    /* If there is a SYS-related log, write it to the proper log file in the
     * OPEN_LOG_DIR.
     */
    if (id >= LOG_SYSBUG_ID)
    {
        write_file((OPEN_LOG_DIR + "/SYS" + LOG_PATH(id)), (text + " " +
            object_name(env) + " (" +
            capitalize(this_player()->query_real_name()) + ")\n" + str +
            "\n"));
        return;
    }

    if (id == LOG_DONE_ID)
    {
        file = this_player()->query_real_name();
    }
    else
    {
        file = creator_object(env);
    }


    file = query_wiz_path(file) + "/log";
    if (file_size(file) != -2)
    {
        mkdir(file);
    }

    file += LOG_PATH(id);
    if (id == LOG_DONE_ID)
    {
        write_file(file, ctime(time()) + ":\n" + str + "\n");
    }
    else
    {
        write_file(file, text + " " + object_name(env) + " (" +
            capitalize(this_player()->query_real_name()) + ")\n" + str + "\n");
    }
}


/*
 * Function name: query_snoop
 * Description:   Tells caller if a user is snooped.
 * Arguments:     snoopee: pointer to a user object
 * Returns:       0 if not snooped, 1 if snooped and caller is lord or lower
 *                object pointer to snooper if caller is arch or higher
 */
mixed query_snoop(object snoopee)
{
    object snooper;
    int type = query_wiz_rank(geteuid(previous_object()));

    if (type >= WIZ_ARCH)
    {
        return interactive_info(snoopee, II_SNOOP_NEXT);
    }

    if (check_snoop_validity(previous_object(), snoopee, 1) &&
        (snooper = (interactive_info(snoopee, II_SNOOP_NEXT))) &&
        (type >= query_wiz_rank(snooper->query_real_name())))
    {
        return 1;
    }

    return 0;
}


/*
 * Function name: query_start_time
 * Description  : Return the time when the game started.
 * Returns      : int - the time.
 */
public int query_start_time()
{
    int theport;
    string game_start;

    theport = 4040; // debug("mud_port");
    if (theport != 0)
    {
        game_start = GAME_START + "." + theport;
        if (file_size(game_start) > 0)
        {
            return get_dir(game_start, GETDIR_DATES)[0];
        }
    }

    /* This value will be wrong if the master has been updated. */
    return object_time(this_object());
}


/*
 * Function name: query_irregular_uptime
 * Description  : The (irregular) uptime after which this game is being
 *                rebooted. This uptime is counted from the start of the
 *                game.
 * Returns      : int - the (irregular) uptime, or 0.
 */
public int query_irregular_uptime()
{
    return irregular_uptime;
}


/*
 * Function name: commune_log
 * Description  : Logs a commune from a mortal.
 * Arguments    : string str  - the message.
 *                int filtered - if true, the message was filtered out.
 */
varargs public void commune_log(string str, int filtered = 0)
{
    log_public((filtered ? "COMMUNE.filtered" : "COMMUNE"),
        sprintf("%s: %-11s: %s", ctime(time()),
        capitalize(this_interactive()->query_real_name()), str));
}


/*
 * Function name: set_runlevel
 * Description  : Set the runlevel. That is the lowest rank of player that
 *                is allowed into the game. Set to WIZ_MORTAL (0) when the
 *                game is open to all players and wizards. This function
 *                may only be called from the normal wizard soul, i.e. from
 *                the 'shutdown' command.
 * Arguments    : int - the runlevel. The argument is not checked for
 *                    validity. That must have been done in the command.
 */
public void set_runlevel(int level)
{
    if (previous_object() != find_object(WIZ_CMD_NORMAL))
    {
        write("Illegal call to set_runlevel(). Call only from wizard soul.\n");
        return;
    }

#ifdef LOG_SHUTDOWN
    log_file("LOG_SHUTDOWN", ctime(time()) + " Runlevel " +
        WIZ_RANK_NAME(level) + " set by " + this_interactive()->query_name() +
        ".\n", -1);
#endif

    runlevel = level;

    save_master();
}


/*
 * Function name: query_runlevel
 * Description  : Returns the runlevel. This is the lowest rank of player
 *                that is allowed into the game. It returns WIZ_MORTAL (0)
 *                when the game is open to all players and wizards.
 * Returns      : int - the runlevel.
 */
public int query_runlevel()
{
    return runlevel;
}


/*
 * Function name: master_shutdown
 * Description  : Perform the final shutdown. This function may only be
 *                called from the armageddon object.
 * Returns      : 1 - Ok, 0 - No shutdown performed.
 */
public int master_shutdown(string reason)
{
    if (MASTER_OB(previous_object()) != ARMAGEDDON)
    {
        return 0;
    }

#ifdef LOG_SHUTDOWN
    log_file("LOG_SHUTDOWN", ctime(time()) + " " + reason, -1);
#endif

    /* This MUST be a this_object()->
     * If it is removed the game wont go down, so hands off!
     */
    this_object()->do_debug("shutdown");
    return 1;
}


/*
 * Function name: request_shutdown
 * Description  : When a wizard wants to shut down the game, this
 *                function is called to invoke Armageddon. The function
 *                should be called from the shutdown command in
 *                WIZ_CMD_NORMAL.
 * Arguments    : string reason - the reason to shut down the game.
 *                int    delay  - the delay in minutes.
 */
public void request_shutdown(string reason, int delay)
{
    string euid    = getwho();
    string shutter = ARMAGEDDON->query_shutter();

    if (sizeof(shutter))
    {
        write("Game is already being shut down by " + shutter + ".\n");
        return;
    }

    if (query_wiz_rank(euid) < WIZ_NORMAL)
    {
        write("Illegal euid. Shutdown rejected.\n");
        return;
    }

    if (!sizeof(reason))
    {
        write("No reason for shutdown. Shutdown rejected.\n");
        return;
    }

    if (reason == "memory_failure")
    {
        /* Arches and keepers can force a memory failure every time. Other
         * wizards can only do so if the memory usage is >= 97%.
         */
        if ((query_wiz_rank(euid) < WIZ_ARCH) &&
            (query_memory_percentage() < 97))
        {
            write("There is no reason to ask for a memory_failure yet.\n");
            return;
        }

        memory_failure();
        return;
    }

    if (reason == "regular_reboot")
    {
        /* Only arches++ may do this. */
        if (query_wiz_rank(euid) < WIZ_ARCH)
        {
            write("There is no reason for you to ask for a regular_reboot.\n");
            return;
        }

        ARMAGEDDON->start_shutdown("The game has been up " +
            CONVTIME(time() - query_start_time()) +
            ", time for a reboot!", 10, ROOT_UID);
        return;
    }

    ARMAGEDDON->start_shutdown(reason, delay, euid);
}


/*
 * Function name: calcel_shutdown
 * Description  : When the wizard has second thoughts and does not want
 *                to shut the game down after all, this function is
 *                called. The function should be called from
 *                WIZ_CMD_NORMAL.
 */
public void cancel_shutdown()
{
    string euid    = getwho();
    string shutter = ARMAGEDDON->query_shutter();
    int    rank    = query_wiz_rank(euid);

    if (mem_fail_flag)
    {
        write("Game is shutting down due to insufficient memory. Cancel " +
            "is not possible.\n");
        return;
    }

    if (rank < WIZ_NORMAL)
    {
        write("Illegal euid. Cancel shutdown rejected.\n");
        return;
    }

    if ((euid != shutter) &&
        (rank < WIZ_ARCH))
    {
        write("You are not allowed to cancel a shutdown by " +
            shutter + ".\n");
        return;
    }

    ARMAGEDDON->cancel_shutdown(euid);
}


/*
 * Function name:  wiz_home
 * Description:    Gives a default 'home' for a wizard, domain or a player
 * Arguments:      wiz: The wizard name.
 * Returns:        A filename for the 'home' room.
 */
string wiz_home(string wiz)
{
    string path;

    logger->debug(" === WIZ_HOME === ");
    if (query_wiz_rank(wiz) == WIZ_MORTAL)
        if (query_domain_number(wiz) < 0)        /* Not even a domain */
            return "";

    path = query_wiz_path(wiz) + "/workroom.c";
    logger->debug("path: %s", path);
    logger->debug("file_size: %d", file_size(path));

    if (file_size(path) <= 0)
        write_file(path, "inherit \"/std/workroom\";\n\n" +
                   "void\ncreate_workroom()\n{\n  ::create_workroom();\n}\n");

    return path;
}


/*
 * Function name: wiz_force_check
 * Description:   Checks if one wizard is allowed to force another
 * Arguments:     forcer: Name of wizard trying to force
 *                forced: Name of wizard being forced
 * Returns:       True if ok
 */
int wiz_force_check(string forcer, string forced)
{
    int rlev;
    int dlev;

    logger->debug(" === WIZ_FORCE_CHECK === ");
    logger->debug("forcer: %s", forcer);
    logger->debug("forced: %s", forced);

    if (forcer == forced)
    {
        return 1;
    }

    rlev = query_wiz_rank(forcer);
    logger->debug("rlev: %d", rlev);

    if ((rlev == WIZ_KEEPER) || (forcer == ROOT_UID) || (forcer == BACKBONE_UID))
    {
        return 1;
    }


    dlev = query_wiz_rank(forced);
    logger->debug("dlev: %d", dlev);

    if ((rlev >= WIZ_ARCH) && (dlev <= WIZ_LORD))
    {
        return 1;
    }

    return 0;
}


/*
 * Function name: set_sanctioned
 * Description:   Set the 'sanctioned' field in the player.
 * Arguments:     wizname - The wizard to set in
 *                map - The 'sanctioned' map to set.
 */
public int set_sanctioned(string wizname, mapping map)
{
    string wname;
    object wiz;

    wname = geteuid(previous_object());

    if ((wname != wizname) && (query_wiz_rank(wname) < WIZ_ARCH))
        return 0;

    wiz = find_player(wizname);
    if (!wiz)
        return 0;

    wiz->set_sanctioned(map);
    return 1;
}


/*
 * Function name: query_banished
 * Description  : Find out whether a name has been banished for use by
 *                new players.
 * Arguments    : string name - the name to check for banishment.
 * Returns      : int 1/0     - true when the name has been banished.
 */
public int query_banished(string name)
{
    if (!sizeof(name))
    {
        return 0;
    }

    /* No need to set the euid. Everyone can see file sizes. */
    name = lower_case(name);
    return (file_size(BANISH_FILE(name)) > 0);
}


/*
 * Function name: banish
 * Description  : Banish a name, info about name, remove a banishment.
 * Arguments    : string name - the name to perform banish operation on.
 *                int what - what to do. 0=info, 1=remove, 2=replace.
 * Returns      : mixed * - ({ ({string})name, (int)time }) or ({})
 *                    the name of the person banishing and the time value at
 *                    which the name was banished.
 */
mixed *banish(string name, int what)
{
    string euid;
    string file;
    mixed *info;

    info = allocate(2);

    if ((previous_object() != find_object(WIZ_CMD_NORMAL)) &&
        (previous_object() != find_object(WIZ_CMD_APPRENTICE)))
    {
        return ({});
    }

    euid = getwho();
    if (!query_wiz_rank(euid))
    {
        return ({});
    }

    name = lower_case(name);
    file = BANISH_FILE(name);

    if (file_size(file) > -1)
    {
        info[0] = read_file(file);
        info[1] = get_dir(file, GETDIR_DATES)[0];
    }

    switch (what)
    {
    case 0: /* Information */
        if (file_size(file) > -1)
        {
            return info;
        }
        break;

    case 1: /* Remove */
        if (file_size(file) > -1)
        {
#ifdef LOG_BANISH
            this_object()->log_syslog(LOG_BANISH,
                sprintf("%s: %-11s unbanishes %s.\n", ctime(time()),
                capitalize(euid), capitalize(name)));
#endif
            rm(file);
            return info;
        }
        break;

    case 2: /* Add */
        if (file_size(file) > -1)
        {
            return info;
        }
#ifdef LOG_BANISH
        this_object()->log_syslog(LOG_BANISH,
            sprintf("%s: %-11s banishes   %s.\n", ctime(time()),
            capitalize(euid), capitalize(name)));
#endif

        /* Make sure the leading directories already exist */
        string dir = "/players";
        if(file_size(dir) == -1) { mkdir(dir); }
        dir += "/banished";
        if(file_size(dir) == -1) { mkdir(dir); }

        write_file(file, euid);
        break;

    default: /* Strange */
        break;
    }

    return ({});
}


/*
 * Function name: do_debug
 * Description  : This function is a front for the efun debug(). You are
 *                only allowed to call debug() through this object because we
 *                need to make some security checks.
 * Arguments    : string icmd - the debug command.
 *                mixed a1    - a possible argument to debug().
 *                mixed a2    - a possible argument to debug().
 *                mixed a3    - a possible argument to debug().
 * Returns      : mixed - the relevant return value for the particular
 *                        debug command.
 */
// varargs mixed
// do_debug(string icmd, mixed a1, mixed a2, mixed a3)
// {
//     string euid = geteuid(previous_object());

//     /* Some debug() commands are not meant to be called by just anybody. Only
//      * 'root' and the administration may call them.
//      */
//     if ((icmd in DEBUG_RESTRICTED))
//     {
//         if ((euid != ROOT_UID) &&
//             (previous_object() != this_object()) &&
//             (previous_object() != find_object(SIMUL_EFUN)) &&
//             (query_wiz_rank(euid) < WIZ_ARCH))
//         {
//             return 0;
//         }
//     }

//     /* In order to get the variables of a certain object, you need to have
//      * the proper euid. If you are allowed to write to the file, you are
//      * also allowed to view its variables.
//      */
//     if ((icmd == "get_variables") &&
//         (!valid_write(object_name(a1), euid, "get_variables")))
//     {
//         return 0;
//     }

//     /* Since debug() returns arrays and mappings by reference, we need to
//      * process the value to make it secure, so people cannot alter it.
//      */
//     return secure_var(debug(icmd, a1, a2, a3));
// }


/*
 * Function name:   send_udp_message
 * Description:     Sends a udp message to arbitrary host, port
 * Arguments:            to_host: Hostname or IP-number
 *                    to_port: Portnumber
 *                    msg:     Message to send
 * Returns:            True if the message is sent. There is of course no
 *                    guarantee it will be received.
 */
int send_udp_message(string to_host, int to_port, string msg)
{
#ifdef UDP_ENABLED
    if (stringp(udp_manager) &&
        previous_object() == find_object(udp_manager))
    {
        return debug("send_udp", to_host, to_port, msg);
    }
#endif
    return 0;
}


/*
 * Function name:  check_memory
 * Description:    Checks with 'debug malloc' if it is time to reboot
 * Arguments:      dodecay - decay xp or nto.
 *                   The limit can be defined in config.h as MEMORY_LIMIT
 */
public void check_memory(int dodecay)
{
    int uptime;

    /* Is the game too big? */
    if (query_memory_percentage() >= 100)
    {
        memory_failure();
    }

    uptime = time() - query_start_time();
#ifdef REGULAR_REBOOT
    /* See whether it is time to reboot the game. The game must have been
     * up at least an hour to reboot, though.
     */
    if (uptime > 3600)
    {
        /* We have to add this 3600 because time() starts counting at 01:00. */
        if ((((time() + 3600) % 86400) / 3600) == REGULAR_REBOOT)
        {
            set_auth(this_object(), "root:root");
            ARMAGEDDON->start_shutdown("It is past " + REGULAR_REBOOT +
                ":00, which is our dayly reboot time!", 10, ROOT_UID);
        }
    }
#endif

#ifdef REGULAR_UPTIME
    if (uptime > irregular_uptime)
    {
        set_auth(this_object(), "root:root");
        ARMAGEDDON->start_shutdown("The game has been up " + CONVTIME(uptime) +
            ", time for a reboot!", 10, ROOT_UID);
    }
#endif

    /* We should add a decay here for the xp stored for each domain. */
    if (dodecay == 1)
    {
        decay_exp();
    }
}


/*
 * Function name: startup_udp
 * Description:   Give the contents of the package to send as startup
 *                message to the mudlist server. This is default if we
 *                have no UDP_MANAGER.
 * Returns:       The message.
 */
string startup_udp()
{
    return
        "||NAME:" + get_mud_name() +
        "||VERSION:" + __VERSION__ +
        "||MUDLIB:" + MUDLIB_VERSION +
        "||HOST:" + __HOST_NAME__ +
        "||PORT:" + "4040" +
        "||PORTUDP:" + "xxxx" +
        "||TIME:" + ctime(time());
}


/*
 * Function name: set_known_muds
 * Description:   The UDP manager can set what muds are known.
 * Returns:       True if set.
 */
int set_known_muds(mapping m)
{
#ifdef UDP_ENABLED
    if (stringp(udp_manager) &&
        previous_object() == find_object(udp_manager))
    {
        known_muds = m;
        save_master();
        return 1;
    }
#endif
    return 0;
}


/*
 * Function name: query_known_muds
 * Description:   Give the currently known muds
 * Returns:       A mapping of information indexed with mudnames
 */
mapping query_known_muds()
{
#ifdef UDP_ENABLED
    if (mappingp(known_muds))
    {
        return known_muds;
    }
#endif
    return 0;
}


/*
 * Function name: query_prevent_shadow
 * Description  : This function will prevent shadowing of this object.
 * Returns      : int 1 - always.
 */
nomask int query_prevent_shadow()
{
    return 1;
}


int file_time(string path)
{
    mixed *v;
    return (sizeof(v = get_dir(path, GETDIR_DATES)) ? v[0] : 0);
}
