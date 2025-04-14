/*
 * /std/workroom.c
 *
 * Generic workroom, just change it to your fit your tastes
 */


#pragma strict_types

inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>
#include <std.h>
#include <libfiles.h>
#include <configuration.h>

#define NAME_WIZ capitalize(getuid())


/*
 * Function name: create_workroom
 * Description  : Called to create the workroom. You may redefine this
 *                function to create your own.
 */
void create_workroom()
{
    set_short(NAME_WIZ + "'s workroom");
    set_long("This is " + NAME_WIZ + "'s workroom.\n");

    add_exit("@@goto_start", "startloc", 0);
    add_prop(ROOM_I_INSIDE, 1);
}


/*
 * Function name: create_room
 * Description:   Create the room
 */
nomask void create_room()
{
    create_workroom();
}


/*
 * Function name: reset_workroom
 * Description  : Called to make the room reset from time to time. You
 *                should redefine this function if you want your workroom
 *                to reset.
 */
public void reset_workroom()
{
}


/*
 * Function name: reset_room
 * Description  : Called to make the room reset from time to time.
 */
nomask void reset_room()
{
    reset_workroom();
}

/*
 * Function name: goto_start
 * Description  : Find and return the default start location of the player
 *                who uses the 'startloc' exit.
 * Returns      : string - the filename of that room.
 */
string goto_start()
{
    return ({string}) this_player()->query_default_start_location();
}
