inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>


void create_room() {
    set_short("West Street");
    set_long("You are on the west street of Newbietown. The street is lined with "
             "small shops and houses. To the east, you can see the town square, "
             "and to the west, the street continues into the distance.\n");

    add_exit("/d/Standard/newbietown/newbietown_square", "east");
    add_exit("/d/Standard/newbietown/arena_of_tests", "west");
}
