inherit "/std/container";

#include <macros.h>
#include <stdproperties.h>
#include <configuration.h>

#include "/std/room/objects.c"

void create_container() {
    set_name("chest");
    set_short("large chest");
    set_long("This is a large wooden chest, it looks like it could hold a lot of items.\n");
    add_prop(OBJ_M_NO_GET, "You can't pick up the chest.\n");
    add_prop(CONT_I_CLOSED, 0);

    add_object("/d/Standard/newbietown/obj/training_sword", 1);

    reset_auto_objects();
    ::create_container();
}
