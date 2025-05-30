#pragma strict_types
#pragma no_inherit
#pragma warn_unused_variables
#pragma warn_dead_code
#pragma warn_applied_functions

#include "/inc/config.h"
#include "/inc/configuration.h"  // For DC_* constants
#include "/inc/driver_hooks.h"    // For H_* constants
#include "/inc/interactive_info.h"
#include "/sys/simul_efuns.h"

// Forward declarations for functions used in config
string load_uid(string file);
string clone_uid(string file);
string create_super(string file);
string create_object(string file);

private object logger;  // We'll initialize this later

#include "/secure/master/config.c"
#include "/secure/master/util.c"

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
    if (err = catch(logger = load_object("/sys/log"))) {
        debug_message("Failed to initialize logger: " + err + "\n");
    } else {
        logger->info("MASTER", "Logger system initialized");
    }
    
    debug_message("=== Master Initialization Complete ===\n");
}

// For all logging calls, add error checking
private void log_message(string type, string msg, varargs mixed *args) {
    if (logger) {
        catch(logger->log_message(type, "MASTER", msg, args...));
    } else {
        debug_message(sprintf("%s: %s\n", type, sprintf(msg, args...)));
    }
}

// ---------- MANDATORY: Error Handling ----------
void log_error(string file, string err, int warn, int line) {
    log_message(warn ? "WARN" : "ERROR", "[%s:%d] %s", file, line, err);
}

void runtime_error(string err, string prog, string obj, int line, mixed culprit, int caught) {
    logger->error("Runtime error in %s (%s) line %d: %s", 
                 obj || "?", prog || "?", line, err);
}

// ---------- MANDATORY: File Security ----------
string valid_read(string path, string uid, string func, object|lwobject obj) {
    return "";  // TODO: Implement proper security
}

string valid_write(string path, string uid, string func, object|lwobject ob) {
    return "";  // TODO: Implement proper security
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

string get_simul_efun() {
    return 0;  // No simul_efun object
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
void dangling_lfun_closure() {
    logger->warn("Dangling lfun closure detected");
}

mixed heart_beat_error(object culprit, string err, string prog, 
                      string curobj, int line, int caught) {
    logger->error("Heart beat error in %O: %s", culprit, err);
    return 0;
}

// ---------- Optional: Security ----------
int privilege_violation(string op, mixed who, mixed arg, mixed arg2, mixed arg3) {
    return 0;  // Deny by default
}

int query_allow_shadow(object victim) {
    return 0;  // No shadowing by default
}

int valid_exec(string name, object ob, object obfrom) {
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
