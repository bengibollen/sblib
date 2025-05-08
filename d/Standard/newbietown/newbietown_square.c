inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>


void create_room() {
    add_prop(OBJ_S_WIZINFO, "@@wizinfo");
    set_short("Main street outside the church");

    set_long("\n"+ "You are on the main street of Newbietown.\n" +
        "The street goes in a east-west direction.\n" +
        "On the north side of the street is the church, which is old and almost in ruins.\n" +
        "Opposite the church lies an inn called 'The Broken Sword'.\n");

    add_exit("/d/Standard/newbietown/church_plaza", "north");
    add_exit("/d/Standard/newbietown/newbie_inn", "south");
    add_exit("/d/Standard/newbietown/street_east", "east");
    add_exit("/d/Standard/newbietown/street_west", "west");

    add_item(({"church"}),"The church is old and almost in ruins.\n");
    add_item(({"inn"}),"The inn is called 'The Broken Sword'.\n");

    add_prop(ROOM_I_INSIDE, 0);
}


void init() {
    ::init();
}


string wizinfo() {
   return ("");
}
