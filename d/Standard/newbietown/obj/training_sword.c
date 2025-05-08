inherit "/std/weapon";

#include <macros.h>
#include <stdproperties.h>

void create_weapon()
{
    set_name("sword");
    set_pname("swords");
    set_short("training sword");
    set_pshort("training swords");
    set_adj("training");
    set_long("This is a training sword, it is not very sharp and is made of wood.\n");

    set_default_weapon();
}

