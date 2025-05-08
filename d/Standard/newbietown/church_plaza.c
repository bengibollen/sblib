inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>


void create_room() {
    add_prop(OBJ_S_WIZINFO, "@@wizinfo");
    set_short("Church plaza of Newbietown");

    set_long("\n"+
	    "You are at the church plaza of Newbietown.\n" +
        "The church to the north is old and almost in ruins. It is way older than the small village south of here.\n" +
        "There's a dry fountain featuring a statue in the middle of the plaza\n" +
        "The village street is to the south.\n");

    add_exit("/d/Standard/start/church", "north");
    add_exit("/d/Standard/newbietown/newbietown_square", "south");

    add_item(({"fountain"}),"It is dry and cracked, a remnant of better days.\n");
    add_item(({"statue"}),"The statue is weathered and worn, depicting an unknown figure.\n" + 
        "There's a small plaque at the base of the statue.\n");
    add_item(({"plaque"}),"The plaque reads: 'Goddess of first steps'\n");
    add_prop(ROOM_I_INSIDE, 0);
}


void init() {
    ::init();
}


string wizinfo() {
   return ("");
}
