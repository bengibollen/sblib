inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>

void create_room() {
    add_prop(OBJ_S_WIZINFO, "@@wizinfo");
    set_short("East Street of Newbietown");

    set_long("\n"+ "You are on the east street of Newbietown, a peaceful cobblestone path\n" +
        "lined with young maple trees. The street stretches east and west, with the\n" +
        "town square visible to the west. To the north, a small garden path winds\n" +
        "between hedges, while the south leads to a cozy marketplace. A narrow\n" +
        "alley branches off to the east, where you can see some residential buildings.\n");

    add_exit("/d/Standard/newbietown/newbietown_square", "west");
    add_exit("/d/Standard/newbietown/garden_path", "north");
    add_exit("/d/Standard/newbietown/marketplace", "south");
    add_exit("/d/Standard/newbietown/residential_alley", "east");

    add_item(({"trees", "maple", "maple trees"}),
        "Young maple trees line the street, their leaves rustling gently in the breeze.");
    add_item(({"cobblestone", "path", "street", "cobblestones"}),
        "The cobblestone street is well-maintained and smooth from years of use.");
    add_item(({"hedges"}),
        "Neatly trimmed hedges border the garden path to the north.");

    add_prop(ROOM_I_INSIDE, 0);
}

void init() {
    ::init();
}

string wizinfo() {
    return ("");
}