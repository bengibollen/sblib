# Master Object Reference

This document provides a reference for the functions defined in `master.c`.

## Functions

### `inaugurate_master(int arg)`

Perform mudlib specific setup of the master.

**Arguments:**
- `arg`: 
  - `0` if the mud just started.
  - `1` if the master is reactivated destructed one.
  - `2` if the master is a reactivated destructed one, which lost all variables.
  - `3` if the master was just reloaded.

**Description:**
This function is called whenever the master becomes fully operational after (re)loading. This doesn't imply that the game is up and running. This function has at least to set up the driverhooks to use. Also, any mudwho or wizlist handling has to be initialized here. Besides that, do whatever you feel you need to do, e.g. set_driver_hook(), or give the master a decent euid.

### `get_master_uid()`

Return the value to be used as uid (and -euid) of a (re)loaded master.

**Description:**
Possible results are in general:
- `"<uid">` -> uid = "<uid>", euid = "<euid>"
In non-strict-euids mode, more results are possible:
- `0` -> uid = 0, euid = 0
- `<num>` -> uid = 'default', euid = 0

### `flag(string arg)`

Evaluate an argument given as option '-f' to the driver.

**Arguments:**
- `arg`: The argument string from the option text '-f<arg>'. If several '-f' options are given, this function will be called sequentially with all given arguments.

**Description:**
This function can be used to pass the master commands via arguments to the driver. This is useful when building a new mudlib from scratch. It is called only when the game is started.

### `epilog(int eflag)`

Perform final actions before opening the game to players.

**Arguments:**
- `eflag`: This is the number of '-e' options given to the parser. Normally it is just 0 or 1.

**Result:**
An array of strings, which traditionally designate the objects to be preloaded with `preload()`, read from the file `/room/init_file`. Any other result is interpreted as 'no object to preload'. The resulting strings will be passed one at the time as arguments to `preload()`.

### `preload(string file)`

Preload a given object.

**Arguments:**
- `file`: The filename of the object to preload, as returned by `epilog()`.

**Description:**
It is task of the `epilog()/preload()` pair to ensure the validity of the given strings (e.g. filtering out comments and blank lines). For preload itself a `load_object(file)` is sufficient, but it should be guarded by a `catch()` to avoid premature blockings. Also it is wise to change the master's euid from master_uid to something less privileged for the time of the preload.

### `connect()`

Handle the request for a new connection.

**Result:**
An login object the requested connection should be bound to, for us a copy of `obj/player.c`.

**Description:**
Note that the connection is not bound yet! The gamedriver will call the lfun `logon()` in the login object after binding the connection to it. That lfun has to return !=0 to succeed.

### `disconnect(object obj)`

Handle the loss of an IP connection.

**Arguments:**
- `obj`: The (formerly) interactive object (player).

**Description:**
This called by the gamedriver to handle the removal of an IP connection, either because the connection is already lost ('netdeath') or due to calls to `exec()` or `remove_interactive()`. The connection will be unbound upon return from this call.

### `remove_player(object player)`

Remove a player object from the game.

**Arguments:**
- `player`: The player object to be removed.

**Description:**
This function is called by the gamedriver to expel remaining players from the game on shutdown in a polite way. If this functions fails to quit/destruct the player, it will be destructed the hard way by the gamedriver. Note: This function must not cause runtime errors.

### `stale_erq(closure callback)`

Notify the loss of the erq demon.

**Arguments:**
- `callback`: The callback closure set for an erq request.

**Description:**
If the erq connection dies prematurely, the driver will call this lfun for every pending request with set callback. This function should notify the originating object that the answer will never arrive. In our case, we simply reattach the default erq.

### `compile_object(string filename)`

Compile a virtual object.

**Arguments:**
- `previous_object()`: The object requesting the virtual object.
- `filename`: The requested filename for the virtual object.

**Result:**
The object to serve as the requested virtual object, or 0.

**Description:**
This function is called if the compiler can't find the filename for an object to compile. The master has now the opportunity to return another which will then serve as if it was compiled from `<filename>`. If the master returns 0, the usual 'Could not load'-error will occur.

### `get_wiz_name(string file)`

Return the author of a file.

**Arguments:**
- `file`: The name of the file in question.

**Result:**
The name of the file's author (or 0 if there is none).

**Description:**
This function is called for maintenance of the wiz-list, to score errors to the right wizard.

### `destruct_environment_of(object ob)`

Handle the destruction of an object's environment.

**Arguments:**
- `ob`: The object whose environment is being destructed.

**Description:**
When an object is destructed, this function is called with every item in that room. We get the chance to save players!

### `move_or_destruct(object what, object to)`

Move an object or destruct it if that is not possible.

**Arguments:**
- `what`: The object to be moved.
- `to`: The destination object.

**Description:**
An error in this function can be very nasty. Note that unlimited recursion is likely to cause errors when environments are deeply nested.

### `prepare_destruct(object ob)`

Prepare the destruction of the given object.

**Arguments:**
- `ob`: The object to destruct.

**Result:**
Return 0 if the object is ready for destruction, any other value will abort the attempt. If a string is returned, an error with the string as message will be issued.

**Description:**
The gamedriver calls this function whenever an object shall be destructed. It expects that this function cleans the inventory of the object, or the destruct will fail. Furthermore, the function could notify the former inventory objects that their holder is under destruction (useful to move players out of rooms which are updated); and it could announce mudwide the destruction (quitting) of players.

### `slow_shut_down(int minutes)`

Schedule a shutdown for the near future.

**Arguments:**
- `minutes`: The desired time in minutes till the shutdown:
  - `6`, if just the user reserve has been put into use.
  - `1`, if the (smaller) system or even the master reserve has been put into use as well.

**Description:**
The gamedriver calls this function when it runs low on memory. At this time, it has freed its reserve, but since it won't last long, the game needs to be shut down. Don't take the 'minutes' as granted remaining uptime, just deduce the urgency of the shutdown from it. The delay is to give the players the opportunity to finish quests, sell their stuff, etc. It is possible that the driver may reallocate some memory after the function has been called, and then run again into a low memory situation, calling this function again.

### `notify_shutdown(string crash_reason)`

Notify the master about an immediate shutdown.

**Arguments:**
- `crash_reason`: If 0, it is a normal shutdown, otherwise it is a crash and `crash_reason` gives a hint at the reason.

**Description:**
The function has the opportunity to perform any cleanup operation, like informing the mudwho server that the mud is down. This can not be done when `remove_player()` is called because the udp connectivity is already gone then. If the gamedriver shuts down normally, this is the last function called before the mud shuts down the udp connections and the accepting socket for new players. If the gamedriver crashes, this is the last function called before the mud attempts to dump core and exit. WARNING: Since the driver is in an unstable state, this function may not be able to run to completion! The following crash reasons are defined: "Fatal Error": an internal sanity check failed.

### `dangling_lfun_closure()`

Handle a dangling lfun-closure.

**Description:**
This is called when the gamedriver executes a closure using a vanished lfun. A proper handling is to raise a runtime error.

### `log_error(string file, string err)`

Announce a compiler-time error.

**Arguments:**
- `file`: The name of file containing the error (it needn't be an object file!).
- `err`: The error message.

**Description:**
Whenever the LPC compiler detects an error, this function is called. It should at least log the error in a file, and also announce it to the active player if it is a wizard.

### `heart_beat_error(object culprit, string err, string prg, string curobj, int line)`

Announce an error in the heart_beat() function.

**Arguments:**
- `culprit`: The object which lost the heart_beat.
- `err`: The error message.
- `prg`: The executed program (might be 0).
- `curobj`: The object causing the error (might be 0).
- `line`: The line number where the error occurred (might be 0).

**Result:**
Return anything != 0 to restart the heart_beat in culprit.

**Description:**
This function has to announce an error in the heart_beat() function of culprit. At time of call, the heart_beat has been turned off. A player should at least get a "You have no heartbeat!" message, a more advanced handling would destruct the offending object and allow the heartbeat to restart. Note that `<prg>` denotes the program actually executed (which might be inherited one) whereas `<curobj>` is just the offending object.

### `runtime_error(string err, string prg, string curobj, int line, mixed culprit)`

Announce a runtime error.

**Arguments:**
- `err`: The error message.
- `prg`: The executed program.
- `curobj`: The object causing the error.
- `line`: The line number where the error occurred.
- `culprit`: `-1` for runtime errors; the object holding the heart_beat() function for heartbeat errors.

**Description:**
This function has to announce a runtime error to the active user, resp. handle a runtime error which occurred during the execution of heart_beat() of `<culprit>`. For a normal runtime error, if the active user is a wizard, it might give him the full error message together with the source line; if the user is a player, it should issue a decent message ("Your sensitive mind notices a wrongness in the fabric of space") and could also announce the error to the wizards online. If the error is a heartbeat error, the heartbeat for the offending `<culprit>` has been turned off. The function itself shouldn't do much, since the lfun `heart_beat_error()` will be called right after this one. Note that `<prg>` denotes the program actually executed (which might be inherited) whereas `<curobj>` is just the offending object for which the program was executed.

### `privilege_violation(string op, mixed who, mixed arg, mixed arg2)`

Validate the execution of a privileged operation.

**Arguments:**
- `op`: The requested operation.
- `who`: The object requesting the operation (filename or object pointer).
- `arg`: Additional argument, depending on `<op>`.
- `arg2`: Additional argument, depending on `<op>`.

**Result:**
- `>0`: The caller is allowed for this operation.
- `0`: The caller was probably misled; try to fix the error.
- `<0`: A real privilege violation; handle it as error.

**Description:**
Privileged operations are:
- `attach_erq_demon`: Attach the erq demon to object `<arg>` with flag `<arg2>`.
- `bind_lambda`: Bind a lambda-closure to object `<arg>`.
- `call_out_info`: Return an array with all call_out informations.
- `erq`: A the request `<arg4>` is to be sent to the erq-demon by the object `<who>`.
- `input_to`: Object `<who>` issues an 'ignore-bang'-input_to() for commandgiver `<arg3>`; the exact flags are `<arg4>`.
- `nomask simul_efun`: Attempt to get an efun `<arg>` via efun:: when it is shadowed by a 'nomask'-type simul_efun.
- `rename_object`: The current object `<who>` renames object `<arg>` to name `<arg2>`.
- `send_imp`: Send UDP-data to host `<arg>`.
- `get_extra_wizinfo`: Get the additional wiz-list info for wizard `<arg>`.
- `set_extra_wizinfo`: Set the additional wiz-list info for wizard `<arg>`.
- `set_extra_wizinfo_size`: Set the size of the additional wizard info in the wiz-list to `<arg>`.
- `set_driver_hook`: Set hook `<arg>` to `<arg2>`.
- `limited`: Execute `<arg>` with reduced/changed limits.
- `set_limits`: Set limits to `<arg>`.
- `set_this_object`: Set this_object() to `<arg>`.
- `shadow_add_action`: Add an action to function `<arg>` from a shadow.
- `symbol_variable`: Attempt to create symbol of a hidden variable of object `<arg>` with index `<arg2>` in the objects variable table.
- `wizlist_info`: Return an array with all wiz-list information.

### `query_allow_shadow(object victim)`

Validate a shadowing.

**Arguments:**
- `previous_object()`: The wannabe shadow.
- `victim`: The object to be shadowed.

**Result:**
Return 0 to disallow the shadowing, any other value to allow it. Destructing the shadow or the victim is another way of disallowing.

**Description:**
This function simply asks the victim if it denies a shadow.

### `query_player_level(string what)`

Check if the player is of high enough level for several things.

**Arguments:**
- `what`: The 'thing' type.

**Result:**
Return 0 to disallow, any other value to allow it.

**Description:**
Types asked for so far are:
- `"error messages"`: Is the player allowed to see error messages (used by the master)? (min-level: 20)
- `"wizard"`: Is the player considered a wizard (used by the mudlib)? (min-level: 20)

### `valid_trace(string what)`

Check if the player is allowed to use tracing.

**Arguments:**
- `what`: The actual action.

**Result:**
Return 0 to disallow, any other value to allow it.

**Description:**
Actions asked for so far are:
- `"trace"`: Is the user allowed to use tracing?
- `"traceprefix"`: Is the user allowed to set a traceprefix? (min-level: 24 for both)

### `valid_exec(string name, object ob, object obfrom)`

Validate the rebinding of an IP connection by usage of efun `exec()`.

**Arguments:**
- `name`: The name of the _program_ attempting to rebind the connection. This is not the object_name() of the object, and has no leading slash.
- `ob`: The object to receive the connection.
- `obfrom`: The object giving the connection away.

**Result:**
Return a non-zero number to allow the action, any other value to disallow it.

**Description:**
Only `obj/master.c` and `secure/login.c` are allowed to do that.

### `valid_query_snoop(object obj)`

Validate if the snoopers of an object may be revealed by usage of the efun `query_snoop()`.

**Arguments:**
- `previous_object()`: The asking object.
- `obj`: The object which snoopers are to be revealed.

**Result:**
Return a non-zero number to allow the action, any other value to disallow it.

**Description:**
Every true wizard can test for a snoop.

### `valid_snoop(object snoopee, object snooper)`

Validate the start/stop of a snoop.

**Arguments:**
- `snoopee`: The victim of the snoop.
- `snooper`: The wannabe snooper, or 0 when stopping a snoop.

**Result:**
Return a non-zero number to allow the action, any other value to disallow it.

**Description:**
It is up to the simul_efun object to start/stop snoops.

### `valid_read(string path, string euid, string fun, object caller)`

Validate a reading/writing file operation.

**Arguments:**
- `path`: The (possibly partial) filename given to the operation.
- `euid`: The euid of the caller (might be 0).
- `fun`: The name of the operation requested.
- `caller`: The calling object.

**Result:**
The full pathname of the file to operate on, or 0 if the action is not allowed. You can also return 1 to indicate that the path can be used unchanged.

**Description:**
The path finally to be used must not contain spaces or '..'. These are the central functions establishing the various file access rights. In this implementation, the main work is done by `obj/player->valid_read(<path>)`.

### `valid_write(string path, string euid, string fun, object caller)`

Validate a writing file operation.

**Arguments:**
- `path`: The (possibly partial) filename given to the operation.
- `euid`: The euid of the caller (might be 0).
- `fun`: The name of the operation requested.
- `caller`: The calling object.

**Result:**
The full pathname of the file to operate on, or 0 if the action is not allowed. You can also return 1 to indicate that the path can be used unchanged.

**Description:**
The path finally to be used must not contain spaces or '..'. These are the central functions establishing the various file access rights. In this implementation, the main work is done by `obj/player->valid_write(<path>)`.

### `make_path_absolute(string str)`

Absolutize a relative filename given to the editor.

**Arguments:**
- `str`: The relative filename (without leading slash).

**Result:**
The full pathname of the file to use. Any non-string result will act as 'bad file name'.

**Description:**
This function is called to convert a relative filename to an absolute path for the editor.

### `save_ed_setup(object who, int code)`

Save individual settings of ed for a wizard.

**Arguments:**
- `who`: The wizard using the editor.
- `code`: The encoded options to be saved.

**Result:**
Return 0 on failure, any other value for success.

**Description:**
This function has to save the given integer into a safe place in the realm of the given wizard, either a file, or in the wizard itself. Be aware of possible security breaches: under !compat, a write_file() should be surrounded by a temporary setting of the master's euid to that of the wizard.

### `retrieve_ed_setup(object who)`

Retrieve individual settings of ed for a wizard.

**Arguments:**
- `who`: The wizard using the editor.

**Result:**
The encoded options retrieved (0 if there are none).

**Description:**
This function retrieves the saved editor settings for the given wizard.

### `get_ed_buffer_save_file_name(string file)`

Return a filename for the ed buffer to be saved into.

**Arguments:**
- `this_player()`: The wizard using the editor.
- `file`: The name of the file currently in the buffer.

**Result:**
The name of the file to save the buffer into, or 0.

**Description:**
This function is called whenever a wizard is destructed/goes netdeath while editing. Using this function, his editing is not done in vain.

### `parse_command_id_list()`

Return generic singular ids.

**Result:**
An array of strings representing generic singular ids.

### `parse_command_plural_id_list()`

Return generic plural ids.

**Result:**
An array of strings representing generic plural ids.

### `parse_command_adjectiv_id_list()`

Return generic adjective ids.

**Result:**
An array of strings representing generic adjective ids.

### `parse_command_prepos_list()`

Return common prepositions.

**Result:**
An array of strings representing common prepositions.

### `parse_command_all_word()`

Return the one(!) 'all' word.

**Result:**
A string representing the 'all' word.

### `verify_create_wizard(object ob)`

Verify that an object is allowed to create a wizard.

**Arguments:**
- `ob`: The object that called create_wizard().

**Result:**
Return 1 if the object is allowed to create a wizard, 0 otherwise.

**Description:**
This function is called for a wizard that has dropped a castle. It verifies that the object is allowed to do this call.

### `master_create_wizard(string owner, string domain, object caller)`

Create a home directory and a castle for a new wizard.

**Arguments:**
- `owner`: The name of the wizard.
- `domain`: The domain of the wizard.
- `caller`: The object calling this function.

**Result:**
The path to the created castle, or 0 on failure.

**Description:**
This function creates directories and files in /players for a new wizard. It is guarded from calls from the wrong places.

### `wiz_decay()`

Decay the 'worth' entry in the wizlist.

**Description:**
This function decays the 'worth' entry in the wizlist by 1% every hour.

### `save_wiz_file()`

Save the wizlist file.

**Description:**
This function saves the wizlist to a file.

### `quota_demon()`

Handle quotas in times of memory shortage.

**Description:**
This function is called during the final phase of a garbage collection if the reserved user area couldn't be reallocated. It has the opportunity to remove some (still active) objects from the game.

### `receive_imp(string host, string msg, int port)`

Handle a received IMP message.

**Arguments:**
- `host`: Name of the host the message comes from.
- `msg`: The received message.
- `port`: The port number from which the message was sent.

**Description:**
This function is called for every message received on the IMP port.

### `get_bb_uid()`

Return the string (or 0) to be used as backbone-euid.

**Description:**
It is just used by process_string() only if no this_object() is present. If strict-euids, the function must exist and return a string.