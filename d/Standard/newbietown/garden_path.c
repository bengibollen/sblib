inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>

void create_room() {
    add_prop(OBJ_S_WIZINFO, "@@wizinfo");
    set_short("Garden Path");

    set_long("\n"+ "This tranquil garden path is a welcome respite from the town streets.\n" +
        "Carefully maintained flower beds line both sides of the path, filled with\n" +
        "colorful blooms and fragrant herbs. A small stone bench sits beneath an\n" +
        "old willow tree, providing a peaceful spot for rest and contemplation.\n" +
        "The path leads back to the street to the south.\n");

    add_exit("/d/Standard/newbietown/street_east", "south");

    add_item(({"flowers", "flower beds", "blooms"}),
        "The flower beds are bursting with color - roses, daisies, and lilies among others.");
    add_item(({"bench", "stone bench"}),
        "A comfortable stone bench, worn smooth by years of use.");
    add_item(({"willow", "tree", "willow tree"}),
        "The old willow tree's branches sway gently, creating patterns of light and shadow.");
    add_item(({"herbs"}),
        "Various aromatic herbs grow here - you can spot mint, thyme, and lavender.");

    add_prop(ROOM_I_INSIDE, 0);
}

void init() {
    ::init();
}

string wizinfo() {
    return ("");
} 