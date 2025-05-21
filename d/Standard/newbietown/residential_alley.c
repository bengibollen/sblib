inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>

void create_room() {
    add_prop(OBJ_S_WIZINFO, "@@wizinfo");
    set_short("Residential Alley");

    set_long("\n"+ "This narrow alley winds between modest residential buildings. Small\n" +
        "gardens and window boxes add splashes of color to the otherwise simple\n" +
        "stone and wooden structures. Laundry lines stretch between upper windows,\n" +
        "and the occasional cat can be seen lounging on a windowsill. The main\n" +
        "street can be accessed to the west.\n");

    add_exit("/d/Standard/newbietown/street_east", "west");

    add_item(({"buildings", "houses"}),
        "Simple but well-maintained buildings made of stone and wood.");
    add_item(({"gardens", "window boxes"}),
        "Small gardens and window boxes filled with colorful flowers and herbs.");
    add_item(({"laundry", "laundry lines"}),
        "Laundry lines stretch between buildings, with various clothes hanging to dry.");
    add_item(({"cats", "cat"}),
        "A few cats can be spotted lounging on windowsills, enjoying the sun.");
    add_item(({"windowsill", "windowsills"}),
        "Wide stone windowsills, perfect for cats to lounge on.");

    add_prop(ROOM_I_INSIDE, 0);
}

void init() {
    ::init();
}

string wizinfo() {
    return ("");
} 