inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>
#include <ss_types.h>
#include <libfiles.h>

#include "/lib/skill_raise.c"

#define SS_RESCUE	67530
#define SS_BASH		67531
#define SS_KICK		67532

void set_up_skills();
void button_a();
int press_console(string str);

void init()
{
    init_skill_raise();
    add_action(#'press_console, "press");

    ::init();
}

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
        "There is also a console of sort here, with some labeled buttons.\n"
    );
    add_exit("/d/Standard/newbietown/street_west", "east");
    add_item("sign", "It is a large sign on the wall.\n");
    add_item("dummies", "They are used for training purposes.\n");
    add_item("equipment", "Various training equipment is available here.\n");
    add_item("shelves", "There are shelves with various weapons and armor here.\n");
    add_item("console", "There are three buttons, labeled A, B, and C.\n");
//    add_cmd_item("console", "press", "@@press_console");
    add_cmd_item("sign", "read", "@@read");
    add_cmd_item("dummies", "inspect", "@@inspect_dummies");
    add_object("/d/Standard/newbietown/obj/chest", 1);

    add_prop(ROOM_I_LIGHT, 1);
    
    create_skill_raise();
    set_up_skills();
}

void set_up_skills()
{
    sk_add_train(SS_WEP_KNIFE, ({"schwong", "swish"}), "Sword", 0, 20, SS_CON, 3);
    sk_add_train(SS_RESCUE, "rescue people when fighting", "rescue", 50, 100 );
    sk_add_train(SS_BASH, "bash an enemy", "bash", 100, 100 );
    sk_add_train(SS_KICK, "kick an enemy", "kick", 100, 100 );
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


int press_console(string str) {
    if (!sizeof(str))
    {
        return 0;
    }

    string *cmd = explode(str, " ");
    if (sizeof(cmd) < 2 || cmd[0] != "button") {
        write("You can't press that.\n");
        return 1;
    }

    switch (cmd[1]) {
    case "A":
        button_a();
        // Add code to start test A
        break;
    case "B":
        write("You press button B. Nothing happens.\n");
        // Add code to start test B
        break;
    case "C":
        write("You press button C. Nothing happens.\n");
        // Add code to start test C
        break;
    default:
        write("You can't press that.\n");
    }
    return 1;
}


void button_a()
{
    if ("/w/debug/sheep" in map(all_inventory(), #'load_name))
    {
        write("You press the button and nothing happens.\n");
        say( QCTNAME(this_player()) + " presses the button and nothing happens.\n");
        return;
    }

    write("You press the button and a sheep appears.\n");
    say( QCTNAME(this_player()) + " presses the button and a sheep appears.\n");
    clone_object("/w/debug/sheep")->move(environment(this_player()));
}


void inspect_dummies(string str) {
    if (str == "dummies") {
        write("The dummies are made of straw and wood, perfect for training.\n");
        // Add code to interact with dummies
    } else {
        write("You can't inspect that.\n");
    }
}
