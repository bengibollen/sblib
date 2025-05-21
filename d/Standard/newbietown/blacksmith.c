inherit "/std/room";
inherit "/obj/lib/shop";

#include <stdproperties.h>
#include <macros.h>
#include <money.h>

void create_room() {
    object blacksmith;
    
    add_prop(OBJ_S_WIZINFO, "@@wizinfo");
    set_short("Blacksmith's Shop");

    set_long("\n"+ "The interior of the blacksmith's shop is warm and dimly lit, dominated by\n" +
        "a large forge against the back wall. Various weapons and tools hang from\n" +
        "hooks on the walls, and a sturdy wooden counter displays some of the\n" +
        "smith's finished work. The air is thick with the scent of hot metal and\n" +
        "coal. A door to the south leads back to the street.\n");

    add_exit("/d/Standard/newbietown/street_west", "south");

    add_item(({"forge"}),
        "The forge burns hot and bright, ready for working metal.");
    add_item(({"counter", "wooden counter"}),
        "A solid wooden counter displays various weapons for sale.");
    add_item(({"tools", "weapons"}),
        "An assortment of hammers, tongs, and finished weapons hang from the walls.");
    add_item(({"hooks"}),
        "Iron hooks line the walls, holding various tools and weapons.");

    add_prop(ROOM_I_INSIDE, 1);

    // Set up the shop functionality
    set_store_room("/d/Standard/newbietown/blacksmith_store");
    init_shop();

    // Create the blacksmith NPC
    blacksmith = clone_object("/std/monster");
    blacksmith->set_name("Goran");
    blacksmith->set_short("Goran the Blacksmith");
    blacksmith->set_long("Goran is a sturdy man with muscular arms and a leather apron. "+
        "His face is friendly despite his imposing build, and he seems eager to help "+
        "new adventurers find suitable weapons.");
    blacksmith->set_level(20);
    blacksmith->set_hp(100);
    MONEY_ADD(blacksmith, 1000);
    
    // Add some basic responses
    blacksmith->add_response("hello", "Goran smiles warmly. 'Welcome to my shop! Looking for something to defend yourself with?'");
    blacksmith->add_response("yes", "Goran nods. 'Take a look at what I have for sale. All quality work, I assure you.'");
    blacksmith->add_response("weapons", "Goran gestures to his wares. 'I've got swords, daggers, and maces - all perfect for beginners.'");
    
    move_object(blacksmith, this_object());

    // Stock the shop with weapons
    stock_shop();
}

void init() {
    ::init();
    init_shop();  // Add shop commands
}

// Custom hook to only allow weapons to be sold here
int shop_hook_allow_sell(object ob) {
    return (int)ob->is_weapon();
}

// Stock the shop with initial inventory
void stock_shop() {
    object store = get_store_object();
    if (!store) return;

    add_weapon(store, "sword", "Short Sword", 25,
        "A well-balanced short sword, perfect for a beginning adventurer.");
    add_weapon(store, "dagger", "Steel Dagger", 15,
        "A sharp steel dagger with a leather-wrapped handle.");
    add_weapon(store, "mace", "Light Mace", 20,
        "A lightweight mace with a sturdy oak handle.");
}

// Helper function to create and add weapons
void add_weapon(object dest, string id, string name, int cost, string desc) {
    object weapon = clone_object("/std/weapon");
    weapon->set_name(id);
    weapon->set_short(name);
    weapon->set_long(desc);
    weapon->set_value(cost);
    weapon->set_weight(1);
    move_object(weapon, dest);
}

string wizinfo() {
    return ("");
} 