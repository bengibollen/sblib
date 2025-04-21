#include "/sys/files.h"

#include "/sys/wizlist.h"
#include "/sys/driver_hook.h"
#include "/sys/objectinfo.h"
#include "/sys/functionlist.h"
#include "/sys/erq.h"
#include "/sys/object_info.h"

#define INIT_FILE "/room/init_file"

#define BACKBONE_WIZINFO_SIZE 5

#define mudwho_init(arg)       0
#define mudwho_connect(ob)     0
#define mudwho_disconnect(ob)  0
#define mudwho_shutdown()      0
#define mudwho_exec(a,b)       0

#define ADD_SLASH(p) "/"+p
#define GETUID(p) getuid(p)
#define TRANSFER(a,b) funcall(symbol_function('transfer), a,b)

#define SIMUL_EFUN_FILE "secure/simul_efun"
#define SPARE_SIMUL_EFUN_FILE "obj/spare_simul_efun"

static private int loading_simul_efun;
void save_wiz_file(); // forward
int query_player_level (string what); // forward


static string _include_dirs_hook(string include_name, string current_file)
{
  string name, part;
  int pos;

  if (file_size(ADD_SLASH(include_name)) >= 0)
    return include_name;
  name = "sys/"+include_name;
  if (file_size(ADD_SLASH(name)) >= 0)
    return name;
  name = "room/"+include_name;
  if (file_size(ADD_SLASH(name)) >= 0)
    return name;
  return 0;
}


//---------------------------------------------------------------------------
static void _move_hook_fun(object item, object dest)
{
  object *others;
  int i;
  string name;

  /* PLAIN:
  if (item != this_object)
      raise_error("Illegal to move other object than this_object()\n");
  */

  logger->debug(" === MOVE HOOK ===");
  logger->debug("Moving object: %s from %s to %s", to_string(item), to_string(environment(item)), to_string(dest));

  if (living(item) && environment(item))
  {
      name = object_info(item, OI_ONCE_INTERACTIVE)
           ? item->query_real_name()
           : object_name(item);
      /* PLAIN: the call to exit() is needed in compat mode only */
      efun::set_this_player(item);
      object env = environment(item);
      env->exit(item);
      if (!item)
          raise_error(sprintf("%O->exit() destructed item %s before move.\n"
                             , env, name));
  }
  else
      name = object_name(item);

  /* This is the actual move of the object. */

  efun::set_environment(item, dest);

  /* Moving a living object will cause init() to be called in the new
   * environment.
   */
  if (living(item)) {
    efun::set_this_player(item);
    dest->init();
    if (!item)
      raise_error(sprintf("%O->init() destructed moved item %s\n", dest, name));
    if (environment(item) != dest)
      return;
  }

  /* Call init() in item once foreach living object in the new environment
   * but only if the item is (still) in the same environment.
   */
  others = all_inventory(dest) - ({ item });
  foreach (object obj : others)
  {
    if (living(obj) && environment(obj) == environment(item)) {
      efun::set_this_player(obj);
      item->init();
    }
    if (!item)
      raise_error(sprintf("item->init() for %O destructed moved item %s\n", obj, name));
  }

  /* Call init() in each of the  objects themselves, but only if item
   * didn't move away already.
   */
  if (living(item)) {
    foreach (object obj : others)
    {
        efun::set_this_player(item); // In case something new was cloned
        if (environment(obj) == environment(item))
            obj->init();
    }
  }

  /* If the destination is alive as well, call item->init() for it. */
  if (living(dest) && item && environment(item) == dest) {
    efun::set_this_player(dest);
    item->init();
  }
}


static string _auto_include_hook(string base_file, string current_file, int sys_include)
{
    // Do nothing for includes.
    if(current_file)
        return 0;

    // Add the light mechanism to every object except the light object itself.
    // And of course ignore master and simul-efun.
    if(base_file[0] != '/')
        base_file = "/" + base_file;

    if(member((["/obj/light.c", "/obj/simul_efun.c", "/obj/spare_simul_efun.c", "/obj/master.c" ]), base_file))
        return 0;

    return "virtual inherit \"/obj/light\";\n";
}


static mixed _load_uids_fun(mixed object_name, object prev)
{
  string * parts;

  parts = explode(object_name, "/");
  if (sizeof(parts) > 2 && parts[0] == "players")
      return parts[1];
  return 1;
}


static mixed _clone_uids_fun(object blueprint, string new_name, object prev)
{
  string creator_name;

  return GETUID(blueprint) || GETUID(prev) || 1;
}


static void _create_fun(object ob, object creator)
{
  closure fun;

  // Create a non-alien closure to reset().
  set_this_object(ob);
  fun = symbol_function("reset", ob);

  // Call it with the creator as the previous object.
  set_this_object(creator);
  funcall(fun, 0);
}


void inaugurate_master(int arg)
{
    if (!arg) {
        if (previous_object() && previous_object() != this_object())
            return;
        set_extra_wizinfo(0, allocate(BACKBONE_WIZINFO_SIZE));
    }

    mudwho_init(arg);

  // Wizlist simulation
  if (find_call_out("wiz_decay") < 0)
    call_out("wiz_decay", 3600);

  set_driver_hook(
        H_MOVE_OBJECT0,
        unbound_lambda( ({'item, 'dest}),
        ({#'_move_hook_fun, 'item, 'dest })
                      )
                 );
  set_driver_hook(
    H_LOAD_UIDS,
    unbound_lambda( ({'object_name}), ({
      #'_load_uids_fun, 'object_name, ({#'previous_object}) })
                  )
  );
  set_driver_hook(
    H_CLONE_UIDS,
    unbound_lambda( ({ /* object */ 'blueprint, 'new_name}), ({
      #'_clone_uids_fun, 'blueprint, 'new_name, ({#'previous_object}) })
                  )
  );
  /* We simulate the old compat mode behavior and call reset()
   * with an argument (0 when creating, 1 when resetting).
   *
   * Non-compat mudlibs usually specify "create" for the H_CREATE_* hooks
   * and "reset" for the H_RESET hook.
   */
  set_driver_hook(
    H_CREATE_SUPER,
    unbound_lambda( ({ 'ob }), ({
      #'_create_fun, 'ob, ({#'this_object}) })
                  )
  );
  set_driver_hook(
    H_CREATE_OB,
    unbound_lambda( ({ 'ob }), ({
      #'_create_fun, 'ob, ({#'this_object}) })
                  )
  );
  set_driver_hook(
    H_CREATE_CLONE,
    unbound_lambda( ({ 'ob }), ({
      #'_create_fun, 'ob, ({#'this_object}) })
                  )
  );
  set_driver_hook(
    H_RESET,
    unbound_lambda(0, ({
      #'funcall, ({#'symbol_function, "reset", ({#'this_object})}), 1 })
                  )
  );
  set_driver_hook(H_CLEAN_UP,     "clean_up");
  set_driver_hook(H_MODIFY_COMMAND,
    ([ "e":"east", "w":"west", "s":"south", "n":"north"
     , "d":"down", "u":"up", "nw":"northwest", "ne":"northeast"
     , "sw":"southwest", "se":"southeast" ]));
  set_driver_hook(H_MODIFY_COMMAND_FNAME, "modify_command");
  set_driver_hook(H_NOTIFY_FAIL, "What?\n");
  set_driver_hook(H_INCLUDE_DIRS, #'_include_dirs_hook);
  set_driver_hook(H_AUTO_INCLUDE, #'_auto_include_hook);
}


mixed get_master_uid()
{
    return 1;
}


void flag(string arg)
{
    string obj, fun, rest;

    if (arg == "shutdown")
    {
      shutdown();
      return;
    }
    if (sscanf(arg, "call %s %s %s", obj, fun, rest) >= 2)
    {
      write(obj+"->"+fun+"(\""+rest+"\") = ");
      write(call_other(obj, fun, rest));
      write("\n");
      return;
    }
    write("master: Unknown flag "+arg+"\n");
}


static mixed current_time;
string *epilog(int eflag)
{
    if (eflag)
        return ({});

    debug_message(sprintf("Loading init file %s\n", INIT_FILE));
    current_time = rusage();
    current_time = current_time[0] + current_time[1];
    return explode(read_file(INIT_FILE), "\n");
}


void preload(string file)
{
    int last_time;

    if (sizeof(file) && file[0] != '#')
    {
        last_time = current_time;
        debug_message(sprintf("Preloading file: %s", file));
        load_object(file);
        current_time = rusage();
        current_time = current_time[0] + current_time[1];
        debug_message(sprintf(" %.2f\n", (current_time - last_time) / 1000.0));
    }
}


void external_master_reload() {
    // Called after a reload of the master on external request.
}


void reactivate_destructed_master(int removed) {
    // Reactivate a formerly destructed master.
}


string get_simul_efun()
{
    mixed error;
    object ob;
  
    error = catch(ob = load_object(SIMUL_EFUN_FILE));
    if (!error)
    {
      ob->start_simul_efun();
      return SIMUL_EFUN_FILE;
    }
    efun::write("Failed to load " + SIMUL_EFUN_FILE + ": "+error);
    error = catch(ob = load_object(SPARE_SIMUL_EFUN_FILE));
    if (!error)
    {
      ob->start_simul_efun();
      return SPARE_SIMUL_EFUN_FILE;
    }
    efun::write("Failed to load " + SPARE_SIMUL_EFUN_FILE + ": "+error);
    efun::shutdown();
    return 0;
}


object connect()
{
    object ob = clone_object("obj/player");
    if (!ob)
    {
        write("Lars says: Couldn't get your body ready ...\n");
        return 0;
    }

#if defined(__TLS__) && defined(TLS_PORT)
    // Figure out what port the interactive is connected to.
    int mud_port = efun::interactive_info(this_interactive(), II_MUD_PORT);
    // If it's the TLS_PORT, then try to initialize TLS.
    if (mud_port == TLS_PORT)
    {
        // reject connection if TLS is not available
        if (!tls_available())
            return 0;
        tls_init_connection(this_object(), "tls_logon", ob);
    }
#endif

    mudwho_connect(ob);
    return ob;
}


void disconnect(object obj)
{
    mudwho_disconnect(obj);
}


void remove_player(object player)
{
    catch(player->quit());
    if (player)
	    destruct(player);
}


void stale_erq(closure callback)
{
    attach_erq_demon("", 0);
}


object compile_object(string filename)
{
    object obj, room;
    mixed vmaster;
    string filepath;
  
    if (filename[0] != '/')
        filename = "/"+filename;

    filepath = implode(explode(filename,"/")[0..<2],"/");
    vmaster = filepath+"/vmaster";

    if (0 <= file_size(vmaster+".c"))
        room = ({object})vmaster->compile_object(explode(filename,"/")[<1]);

    if (!room && 0 <= file_size(filepath+".c"))
        room = ({object})filepath->compile_object(explode(filename,"/")[<1]);

    return room;
}


mixed include_file(string file, string compiled_file, int sys_include)
{
    // Return the full pathname for an included file.
    return 0;
}


mixed inherit_file(string file, string compiled_file)
{
    // Return the full pathname for an inherited object.
    return 0;
}


string get_wiz_name(string file)
{
    string name, rest;

    if (sscanf(file, "players/%s/%s", name, rest) == 2) {
	return name;
    }
    return 0;
}


void destruct_environment_of(object ob)
{
    mixed error;

    if (!interactive(ob))
	return;
    tell_object(ob, "Everything you see is disolved. Luckily, you are transported somewhere...\n");
    if (error = catch(ob->move_player("is transfered#room/void"))) {
	write(error);
	if (error = catch(move_object(ob, "room/void"))) {
	    object new_player;

	    write(error);
	    new_player = clone_object("obj/player");
	    if (!function_exists("replace_player", new_player)) {
		destruct(new_player);
		return;
	    }
	    exec(new_player, ob);
	    if (error = catch(new_player->replace_player(ob, "room/void"))) {
		write(error);
	    }
	}
    }
}


void move_or_destruct(object what, object to)
{
    int res;

    /* PLAIN: the following loop is for compat mode only */
    do {
        if (catch( res = TRANSFER(what, to) )) res = 5;
        if ( !(res && what) ) return;
    } while( (res == 1 || res == 4 || res == 5) && (to = environment(to)) );
    /* PLAIN: native muds make this
    if (!catch(what->move(to, 1)))
        return;
    */

    /*
     * Failed to move the object. Therefore it is destroyed.
     */
    destruct(what);
}


string printf_obj_name(object ob)
{
    string tmp;

    tmp = ({string})ob->query_real_name();
    if (stringp(tmp) && tmp != "")
	return tmp;
    tmp = ({string})ob->query_name();
    if (stringp(tmp) && tmp != "")
	return tmp;
    return "UID:" + getuid(ob);
}


mixed prepare_destruct(object ob)
{
    object super;
    int i;
    object sh, next;

    /* Remove all pending shadows */
    if (!object_info(ob, OI_SHADOW_PREV))
        for (sh = object_info(ob, OI_SHADOW_NEXT); sh; sh = next) {
            next = efun::object_info(sh, OI_SHADOW_NEXT);
            funcall(bind_lambda(#'unshadow, sh)); /* avoid deep recursion */
            destruct(sh);
        }

    super = environment(ob);

    if (!super) {
	object item;

	while ( item = first_inventory(ob) ) {
	    destruct_environment_of(item);
	    if (item && environment(item) == ob) destruct(item);
	}
    } else {
	while ( first_inventory(ob) )
	    move_or_destruct(first_inventory(ob), super);
    }

    if (interactive(ob))
        disconnect(ob);

    return 0; /* success */
}


void quota_demon()
{
    // Handle quotas in times of memory shortage.
}


void receive_udp(string host, string msg, int port)
{
    // Handle a received IMP message.
}


void slow_shut_down(int minutes)
{
    filter(users(), #'tell_object,
      "Game driver shouts: The memory is getting low !\n");
    "obj/shut"->shut(minutes);
}


void notify_shutdown(string crash_reason)
{
    if (previous_object() && previous_object() != this_object())
        return;
    if (!crash_reason)
        filter(users(), #'tell_object,
          "Game driver shouts: LPmud shutting down immediately.\n");
    else
        filter(users(), #'tell_object,
          "Game driver shouts: PANIC! "+ crash_reason+"!\n");
    save_wiz_file();
    mudwho_shutdown();
}


void dangling_lfun_closure()
{
    raise_error("dangling lfun closure\n");
}


void log_error(string file, string err, int warn, int line)
{
    string name;

    name = get_wiz_name(file);
    if (name == 0)
	name = "log";
    write_file("/log/"+name, err);
}


mixed heart_beat_error(object culprit, string err, string prg, string curobj, int line)
{
    if ( interactive(culprit) ) {
        tell_object(
          culprit,
          "Game driver tells you: You have no heart beat !\n"
        );
        }
        return 0; /* Don't restart */
}


void runtime_error(string err, string prg, string curobj, int line)
{
    if (this_player() && interactive(this_player()))
        catch( write(
        query_player_level("error messages") ?
            curobj ?
            err +
            "program: " + prg +
            ", object: " + curobj +
            " line " + line + "\n"
            :
            err
        :
            "Your sensitive mind notices a wrongness in the fabric of space.\n"
        ) );
}


void runtime_warning(string msg, string curobj, string prg, int line)
{
    // Announce a runtime warning.
}


int privilege_violation(string op, mixed who, mixed arg, mixed arg2)
{
    /* This object and the simul_efun objects may do everything */
    if (who == this_object()
        || who == find_object(SIMUL_EFUN_FILE)
        || who == find_object(SPARE_SIMUL_EFUN_FILE))
           return 1;
   
       switch(op) {
         case "erq":
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
       return 0;
}


int query_allow_shadow(object victim)
{
    if (object_info(victim, OINFO_MEMORY)[OIM_NO_SHADOW])
        return 0;
    return !victim->query_prevent_shadow(previous_object());
}


int valid_exec(string name, object ob, object obfrom)
{
    switch(name) {
        case "secure/login.c":
        case "obj/master.c":
      if (!interactive(ob)) {
              mudwho_exec(obfrom, ob);
          return 1;
          }
      }
  
      return 0;
}


int valid_query_snoop(object obj)
{
    return this_player()->query_level() >= 22;
}


int valid_snoop(object snoopee, object snooper)
{
    /* PLAIN:
    if (!geteuid(previous_object()))
        return 0;
    */
    if (object_name(previous_object()) == get_simul_efun())
        return 1;

    return 0;
}



int|string valid_read(string path, string euid, string fun, object caller)
{
    string user;

    switch ( fun ) {
        case "restore_object": return 1;
        case "ed_start":
            if ( previous_object() && previous_object() != this_player() )
                return 0;
            if (!path) {
                /* request for file with last error */
                mixed *error;

                error =
                  get_error_file(({string})this_player()->query_real_name());
                if (!error || error[3]) {
                    write("No error.\n");
                    return 0;
                }
                write(error[0][1..]+" line "+error[1]+": "+error[2]+"\n");
                return ADD_SLASH(error[0]);
            }
            if (path[0] != '/')
                path = "/"+path;
        case "read_file":
        case "read_bytes":
        case "file_size":
        case "get_dir":
        case "do_rename":
            if (caller == this_object()) return 1;
        case "tail":
        case "print_file":
        case "make_path_absolute": /* internal use, see below */
            set_this_object(caller);
            if( this_player() && interactive(this_player()) ) {
                path = ({string})this_player()->valid_read(path);
                if (!stringp(path)) {
                    write("Bad file name.\n");
                    return 0;
                }
                return ADD_SLASH(path);
            }
            path = ({string})"obj/player"->valid_read(path);
            if (stringp(path))
                return ADD_SLASH(path);
            return 0;
    }
    /* if a case failed to return a value or the caller function wasn't
     * recognized, we come here.
     * The default returned zero indicates deny of access.
     */
    return 0;
}


int|string valid_write(string path, string euid, string fun, object caller)
{
    string user;

    if (path[0] == '/' && path != "/")
        path = path[1..];

    switch ( fun ) {
    case "objdump":
        if (path == "OBJ_DUMP") return path;
        return 0;

    case "opcdump":
        if (path == "OPC_DUMP") return path;
        return 0;

    case "save_object":
        if ( user = GETUID(previous_object()) ) {
            if ( path[0 .. sizeof(user)+7] == "players/" + user
             &&  sscanf(path, ".%s", user) == 0)
                return ADD_SLASH(path);
        } else {
            user = efun::object_name(previous_object());
#ifndef __COMPAT_MODE__
            user = user[1..];
#endif
            if ( user[0..3] == "obj/"
             ||  user[0..4] == "room/"
             ||  user[0..3] == "std/"  )
                return ADD_SLASH(path);
        }
        return 0; /* deny access */
    default:
        return 0; /* deny access */
    case "write_file":
        if (caller == this_object()) return 1;
        if (path[0..3] == "log/"
         && !(   sizeof(regexp(({path[4..33]}), "/"))
              || path[4] == '.'
              || sizeof(path) > 34
            ) ) {
            return ADD_SLASH(path);
        }
        break;
    case "ed_start":
        if (path[0] != '/')
            path = "/"+path;
        break;
    case "rename_from":
    case "rename_to":
        if ((   efun::object_name(caller) == SIMUL_EFUN_FILE
             || efun::object_name(caller) == SPARE_SIMUL_EFUN_FILE)
         && path[0..3] == "log/"
         && !(   sizeof(regexp(({path[4..33]}), "/"))
              || path[4] == '.'
              || sizeof(path) > 34
            ) ) {
            return 1;
        }
    case "mkdir":
    case "rmdir":
    case "write_bytes":
    case "remove_file":
        if (caller == this_object()) return 1;
    }

    set_this_object(caller);
    if( this_player() && interactive(this_player()) )
    {
        path = ({string})this_player()->valid_write(path);
        if (!stringp(path)) {
            write("Bad file name.\n");
            return 0;
        }
        return ADD_SLASH(path);
    }
    path = ({string})"obj/player"->valid_write(path);
    if (stringp(path))
        return ADD_SLASH(path);

    return 0;
}


string make_path_absolute(string str)
{
    return valid_read(str,0,"make_path_absolute", this_player());
}


int save_ed_setup(object who, int code)
{
    string file;

    if (!intp(code))
	return 0;
    file = "/players/" + lower_case(({string})who->query_name()) + "/.edrc";
    rm(file);
    return write_file(file, code + "");
}


int retrieve_ed_setup(object who)
{
    string file;
    int code;

    file = "/players/" + lower_case(({string})who->query_name()) + "/.edrc";
    if (file_size(file) <= 0)
	return 0;
    sscanf(read_file(file), "%d", code);
    return code;
}


string get_ed_buffer_save_file_name(string file)
{
    string *file_ar;
    string path;

    path = "/players/"+this_player()->query_real_name()+"/.dead_ed_files";
    if (file_size(path) == -1) {
        mkdir(path);
    }
    file_ar=explode(file,"/");
    file=file_ar[sizeof(file_ar)-1];
    return path+"/"+file;
}


string *parse_command_id_list()
{
    return ({ "one", "thing" });
}


string *parse_command_plural_id_list()
{
    return ({ "ones", "things", "them" });
}


string *parse_command_adjectiv_id_list()
{
    return ({ "iffish" });
}


string *parse_command_prepos_list()
{
    return ({ "in", "on", "under", "behind", "beside" });
}


string parse_command_all_word()
{
    // Return the one(!) 'all' word.
    return "all";
}

static void wiz_decay()
{
    mixed *wl;
    int i;

    wl = wizlist_info();
    for (i=sizeof(wl); i--; ) {
        set_extra_wizinfo(wl[i][WL_NAME], wl[i][WL_EXTRA] * 99 / 100);
    }
    call_out("wiz_decay", 3600);
}


void save_wiz_file()
{
#ifdef __WIZLIST__
    rm(__WIZLIST__);
    write_file(
      __WIZLIST__,
      implode(
        map(wizlist_info(),
          lambda(({'a}),
            ({#'sprintf, "%s %d %d\n",
              ({#'[, 'a, WL_NAME}),
              ({#'[, 'a, WL_COMMANDS}),
              ({#'[, 'a, WL_EXTRA})
            })
          )
        ), ""
      )
    );
#endif
}


int verify_create_wizard (object ob)
{
    int dummy;

    if (sscanf(object_name(ob), "room/port_castle#%d", dummy) == 1
      || sscanf(object_name(ob), "global/port_castle#%d", dummy) == 1)
	return 1;
    return 0;
}


string master_create_wizard(string owner, string domain, object caller)
{
    string def_castle;
    string dest, castle, wizard;
    object player;

    /* find_player() is a simul_efun. Resolve it at run time. */
    player = funcall(symbol_function('find_player),owner);
    if (!player)
	return 0;
    if (!verify_create_wizard(caller)) {
	tell_object(player, "That is an illegal attempt!\n");
	return 0;
    }
    if (caller != previous_object()) {
	tell_object(player, "Faked call!\n");
	return 0;
    }
    wizard = "/players/" + owner;
    castle = "/players/" + owner + "/castle.c";
    if (file_size(wizard) == -1) {
	tell_object(player, "You now have a home directory: " +
		    wizard + "\n");
	mkdir(wizard);
    }
    dest = object_name(environment(player));
    def_castle = "#define NAME \"" + owner + "\"\n#define DEST \"" +
	dest + "\"\n" + read_file("/room/def_castle.c");
    if (file_size(castle) > 0) {
	tell_object(player, "You already had a castle !\n");
    } else {
	/* The master object can do this ! */
	if (write_file(castle, def_castle)) {
	    tell_object(player, "You now have a castle: " + castle + "\n");
	    if (!write_file("/room/init_file", castle[1..] + "\n"))
		tell_object(player, "It couldn't be loaded automatically!\n");
	} else {
	    tell_object(player, "Failed to make castle for you!\n");
	}
    }
    return castle;
}
