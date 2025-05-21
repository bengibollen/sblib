inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>

void create_room() {
    set_short("Blacksmith's Storeroom");
    set_long("This is the storeroom for the blacksmith's shop.\n");
    add_prop(ROOM_I_LIGHT, 1);
    add_prop(ROOM_I_INSIDE, 1);
    add_prop(ROOM_I_NO_CLEANUP, 1);  // Prevent cleanup of stored items
}

void init() {
    ::init();
} 