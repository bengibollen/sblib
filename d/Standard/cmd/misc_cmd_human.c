/*
 * Copyright (c) 1991 Chalmers Computer Society
 *
 * This code may not be copied or used without the written permission
 * from Chalmers Computer Society.
 */

/*

  misc_cmd_human.c

  This is the default object defining misc commands.
  Do not inherit this object, use the objects under /std

*/

#pragma strict_types
#pragma no_clone

inherit "/std/command_driver";

#include <macros.h>
#include <std.h>
#include <stdproperties.h>
#include <configuration.h>

void create()
{
    configure_object(this_object(), OC_EUID, getuid(this_object()));
}


/*
 * What souls to use as misc souls
 */
string *replace_soul()
{
    log_debug("replace_soul()");
    log_debug("MASTER: " + MASTER);
    return
	({
	    MASTER,
	    "/std/cmd/live/things",
	    "/std/cmd/live/social",
	    "/std/cmd/live/speech",
	    "/std/cmd/live/state",
	    "/std/cmd/live/items",
	    "/std/cmd/live/info"
	});
}


/*
 * Return a proper name of the soul in order to get a nice printout.
 */
string get_soul_id()
{
    string *st;

    st = explode(object_name(this_object()), "/");
    return st[sizeof(st) - 1];
}


/*
 * This is a command soul.
 */
int query_cmd_soul()
{
    return 1;
}


/*
 * The list of verbs and functions. Please add new in alphabetical order.
 */
mapping query_cmdlist()
{
    return ([
/*	"command" : "function",	*/
	]);
}


/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the
 * same order as in the function name list.
 * **************************************************************************/
