inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>

void create_room() {
    add_prop(OBJ_S_WIZINFO, "@@wizinfo");
    set_short("West Street of Newbietown");

    set_long("\n"+ "The cobblestone street here is marked by the rhythmic clanging of metal\n" +
        "on metal from the blacksmith's shop on the north side. Smoke rises from\n" +
        "the chimney, and the warm glow of the forge can be seen through the open\n" +
        "doorway. The street continues east to the town square, while to the west\n" +
        "you can see the entrance to the Arena of Tests. You can enter the\n" +
        "blacksmith's shop to the north.\n");

    add_exit("/d/Standard/newbietown/newbietown_square", "east");
    add_exit("/d/Standard/newbietown/blacksmith", "north");
    add_exit("/d/Standard/newbietown/arena_of_tests", "west");

    add_item(({"shop", "blacksmith", "blacksmith's shop"}),
        "The blacksmith's shop is a sturdy stone building with a large wooden door. The sound of hammering comes from inside.");
    add_item(({"chimney"}),
        "A stone chimney rises from the roof, wisps of smoke curling into the sky.");
    add_item(({"forge", "glow"}),
        "Through the doorway, you can see the orange glow of the forge.");
    add_item(({"street", "cobblestone"}),
        "The cobblestone street is well-worn from years of traffic.");
    add_item(({"arena", "arena of tests"}),
        "The Arena of Tests lies to the west, where adventurers can test their combat skills.");

    add_prop(ROOM_I_INSIDE, 0);
}

void init() {
    ::init();
}

string wizinfo() {
    return ("");
}
