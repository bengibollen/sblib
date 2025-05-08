inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>

void create_room() {
    set_short("The Arena of Tests");
    // With tests, it actually refer to betatest of the fighting aspects of the game.
    // Some tounge in cheek descriptoins here to make it a bit funny.
    set_long(
        "You are in the Arena of Tests. You are suddenly somehow in the middle of\n" +
        "a large arena. You see a lot of dummies and other training equipment.\n" +
        "You can see a lot of people running around, and some are even fighting.\n" +
        "You can see a large sign on the wall.\n" +
        "There are shelves with various weapons and armor here.\n" +
        "There are also a console of sort here, with some labeled buttons.\n"
    );
    add_exit("/d/Standard/newbietown/main_street_outside_church", "south");
    add_item("sign", "It is a large sign on the wall.\n");
    add_item("dummies", "They are used for training purposes.\n");
    add_item("equipment", "Various training equipment is available here.\n");
    add_item("shelves", "There are shelves with various weapons and armor here.\n");
    add_item("console", "There are three buttons, labeled A, B, and C.\n");
    add_cmd_item("press", "console", "@@press_console");
    add_cmd_item("read", "sign", "@@read");
    add_cmd_item("inspect", "dummies", "@@inspect_dummies");
    add_object("/d/Standard/newbietown/obj/chest", 1);

}

void read(string str) {
    if (str == "sign") {
        write("The sign reads:\n" +
              "Welcome to the Arena of Tests!\n" +
              "Here you can test your skills and abilities.\n" +
              "Press the buttons on the console to start a test.\n");
    } else {
        write("You can't read that.\n");
    }
}
void press_console(string str) {
    if (str == "A") {
        write("You press button A. A test begins!\n");
        // Add code to start test A
    } else if (str == "B") {
        write("You press button B. A test begins!\n");
        // Add code to start test B
    } else if (str == "C") {
        write("You press button C. A test begins!\n");
        // Add code to start test C
    } else {
        write("You can't press that.\n");
    }
}
void inspect_dummies(string str) {
    if (str == "dummies") {
        write("The dummies are made of straw and wood, perfect for training.\n");
        // Add code to interact with dummies
    } else {
        write("You can't inspect that.\n");
    }
}
