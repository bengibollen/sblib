inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>

void create_room() {
    add_prop(OBJ_S_WIZINFO, "@@wizinfo");
    set_short("Newbietown Marketplace");

    set_long("\n"+ "The marketplace is a lively area where local merchants gather to trade.\n" +
        "Wooden stalls are arranged in neat rows, displaying various goods from\n" +
        "fresh produce to handcrafted items. The air is filled with the chatter\n" +
        "of haggling customers and the occasional call of a merchant advertising\n" +
        "their wares. The main street lies to the north.\n");

    add_exit("/d/Standard/newbietown/street_east", "north");

    add_item(({"stalls", "wooden stalls"}),
        "The wooden stalls are sturdy and well-built, each with its own colorful awning.");
    add_item(({"goods", "wares", "produce"}),
        "Various goods are on display - fruits, vegetables, bread, and crafted items.");
    add_item(({"merchants", "vendors"}),
        "Local merchants busy themselves arranging their goods and dealing with customers.");
    add_item(({"awning", "awnings"}),
        "Colorful awnings provide shade for the merchants and their goods.");

    add_prop(ROOM_I_INSIDE, 0);
}

void init() {
    ::init();
}

string wizinfo() {
    return ("");
} 