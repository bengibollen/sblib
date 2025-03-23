#pragma strict_types
#pragma save_types

#include <player.h>
#include <input_to.h>
#include <configuration.h>
#include <macros.h>

inherit "/std/living";

#include "/std/player/cmd_sec.c"

private string name;          // Player's name
private int state;           // Current player state
private int level;          // Player level

private void show_entrance();

public void create_living() {
    log_info("Player object created");
    log_debug("Object name: %s", object_name(this_object()));

    configure_object(this_object(), OC_COMMANDS_ENABLED, 1);
    state = PLAYER_STATE_LOADING;
}

public void initialize(string player_name) {
    log_debug("Initializing player with name: %s", player_name);
    name = player_name;
    state = PLAYER_STATE_PLAYING;
//    configure_object(this_object(), OC_COMMANDS_ENABLED, 1);
    log_info("Player %s initialized", name);
    log_debug("Object name: %s", object_name(this_object()));

    show_entrance();
}

private void show_entrance() {
    write("\nWelcome to the game, " + capitalize(name) + "!\n");
//    command("look");  // Show initial room description
}

// Query functions
public varargs string query_name() { return name; }
public int query_level() { return level; }
public int query_state() { return state; }

// Save/restore functions - to be implemented
public int save_player() {
    // TODO: Implement save functionality
    return 1;
}

public int restore_player() {
    // TODO: Implement restore functionality
    return 1;
}

public void player_startup() {
    log_debug("Player startup initiated for: %s", query_name());
    cmdhooks_reset();
    cmd_sec_reset();


    /* Get the soul commands */
//    this_object()->load_command_souls();
    command("look");
    say(QCNAME(this_object()) + " enters the game.\n");
}

public void catch_tell(string message) {
    write(message);
}   

public void catch_msg(string message) {
    write(process_string(message));
}
