#pragma strict_types
#pragma no_inherit
#pragma warn_unused_variables
#pragma warn_dead_code
#pragma warn_applied_functions

inherit "/secure/simul/security";

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

// Forward declarations for functions used in config
string load_uid(string file);
string clone_uid(string file);
string create_super(string file);
string create_object(string file);
static void save_master();

private object logger;  // We'll initialize this later

#include "/secure/master/error_handling.c"
#include "/secure/master/config.c"
//#include "/secure/master/fob.c"
//#include "/secure/master/sanction.c"

#define DEBUG_RESTRICTED ( ({ "mudstatus", "swap", "shutdown", "send_udp" }) )

// Global variables
string mudlib_version = "SBLib v0.1";

// ---------- MANDATORY: Initialization Functions ----------
string get_master_uid() {
    return "ROOT";
}

void inaugurate_master(int arg)
{ 
    // Use debug_message until logger is ready
    debug_message("\n=== Master Initialization Starting ===\n");
    debug_message(sprintf("Master object loaded (arg: %d)\n", arg));
    
    setup_all();
    
    // Now it's safe to initialize the logger
    string err;
    if (err = catch(logger = load_object("/lib/log"))) {
        debug_message("Failed to initialize logger: " + err + "\n");
    } else {
        logger->info("Logger system initialized");
    }
    
    debug_message("=== Master Initialization Complete ===\n");
}

// For all logging calls, add error checking
private void log_message(string type, string msg, varargs mixed *args) {
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
string valid_read(string path, string uid, string func, object|lwobject obj) {
    return path;  // TODO: Implement proper security
}

string valid_write(string path, string uid, string func, object|lwobject ob) {
    return path;  // TODO: Implement proper security
}

// ---------- MANDATORY: File Resolution ----------
mixed inherit_file(string file, string compiled_file) {
    if (file[0] != '/') file = "/" + file;
    return file;
}

mixed include_file(string file, string compiled_file, int sys_include) {
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

public object connect() {
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
string get_bb_uid() {
    return "BACKBONE";
}

// ---------- MANDATORY: UID Management ----------
string load_uid(string file) {
    return ROOT_UID;  // For now, everything gets ROOT uid
}

string clone_uid(string file) {
    return ROOT_UID;  // For now, everything gets ROOT uid
}

string create_super(string file) {
    return ROOT_UID;  // For now, everything gets ROOT uid
}

string create_object(string file) {
    return ROOT_UID;  // For now, everything gets ROOT uid
}

// ---------- Optional: Initialization ----------
void flag(string arg) {
    // Called for each -f flag passed to driver
}

string *epilog(int eflag) {
    return ({ });  // No objects to preload by default
}

void preload(string file) {
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
void disconnect(object obj, string remaining) {
    // Called when user disconnects
}

void remove_player(object user) {
    // Called when player object is removed
}

void stale_erq(closure callback) {
    // Called when ERQ connection is lost
}

// ---------- Optional: Runtime Support ----------
object compile_object(string filename) {
    return 0;  // No virtual objects
}

string get_wiz_name(string file) {
    return "NOBODY";
}

string printf_obj_name(object obj) {
    return sprintf("<%O>", obj);
}

mixed prepare_destruct(object obj) {
    return 0;  // Allow destruction
}

void quota_demon() {
    // Memory quota handling
}

void receive_imp(string host, string msg, int port) {
    // IMP message handling
}

void slow_shut_down(int minutes) {
    // Gradual shutdown handling
}

void notify_shutdown() {
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
int privilege_violation (string op, mixed who, mixed arg, mixed arg2, mixed arg3)

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
{
    /* This object and the simul_efun objects may do everything */
    if (who == this_object()
     || who == find_object(SIMUL_EFUN))
        return 1;

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
	    return -1;
	}
      default:
	return -1; /* Make this violation an error */
    }
}

int query_allow_shadow(object victim) {
    return 0;  // No shadowing by default
}

int valid_exec(string name, object ob, object obfrom) {
    switch(name) {
        case "secure/login.c":
        case "secure/master.c":
      if (interactive(obfrom) && !interactive(ob)) {
           return 1;
          }
      }
    return 0;  // No exec() by default
}

int valid_query_snoop(object obj) {
    return 0;  // No snoop queries by default
}

int valid_snoop(object snoopee, object snooper) {
    return 0;  // No snooping by default
}

// ---------- Optional: Editor Support ----------
string make_path_absolute(string str) {
    return (str[0] == '/' ? str : "/" + str);
}

int save_ed_setup(object who, int code) {
    return 0;  // No saved editor settings
}

int retrieve_ed_setup(object who) {
    return 0;  // No saved editor settings
}

string get_ed_buffer_save_file_name(string file) {
    return "/tmp/ed." + file;
}

// ---------- Optional: parse_command Support ----------
string *parse_command_id_list() {
    return ({ });
}

string *parse_command_plural_id_list() {
    return ({ });
}

string *parse_command_adjectiv_id_list() {
    return ({ });
}

string *parse_command_prepos_list() {
    return ({ });
}

string parse_command_all_word() {
    return "all";
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
varargs mixed do_debug(string icmd, mixed a1, mixed a2, mixed a3)
{
    string euid = geteuid(previous_object());

    /* Some debug() commands are not meant to be called by just anybody. Only
     * 'root' and the administration may call them.
     */
    if (IN_ARRAY(icmd, DEBUG_RESTRICTED))
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
//    return secure_var(debug(icmd, a1, a2, a3));
    return "";
}

/*
 * Function name: save_master
 * Description  : This function saves the master object to a file.
 */
static void save_master()
{
    // set_auth(this_object(), "root:root");

    // save_object(SAVEFILE);
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
        configure_object(previous_object(), OC_EUID, "0:0");
    }
}

