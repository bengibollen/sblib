#pragma strict_types
#pragma save_types

#include <player.h>
#include <input_to.h>
#include <configuration.h>

inherit "/std/living";

private string name;          // Player's name
private int state;           // Current player state
private int level;          // Player level
private mapping commands;    // Available commands
private object logger;      // Logger instance

private void show_entrance();

public void create_living() {
    logger = load_object("/lib/log");
    logger->info("Player object created");
    logger->debug("Object name: %s", object_name(this_object()));
    configure_object(this_object(), OC_COMMANDS_ENABLED, 1);
    commands = PLAYER_COMMANDS;  // Load basic commands
    state = PLAYER_STATE_LOADING;
}

public void initialize(string player_name) {
    name = player_name;
    state = PLAYER_STATE_PLAYING;
//    configure_object(this_object(), OC_COMMANDS_ENABLED, 1);
    logger->info("Player %s initialized", name);
    logger->debug("Object name: %s", object_name(this_object()));
    show_entrance();
}

private void show_entrance() {
    write("\nWelcome to the game, " + capitalize(name) + "!\n");
    command("look");  // Show initial room description
}

// Command processing
public int command(string cmd) {
    string verb, args;
    
    if (!cmd || cmd == "") return 0;
    
    // Split command into verb and arguments
    if (sscanf(cmd, "%s %s", verb, args) != 2) {
        verb = cmd;
        args = "";
    }
    
    verb = lower_case(verb);
    
    // Check if command exists and call it
    if (member(commands, verb)) {
        int result;
        string command = commands[verb];
        string ob = object_name(this_object());

        result = ({int})this_object()->(commands[verb])(args);
        // Call the command and ensure we get an int back
        // result = (int)this_object()->(command)(args);
        if (intp(result)) return result;

        logger->warn("Command %s in %s did not return an int", command, ob);
        return 0;  // Non-integer results are treated as failure
    }
    
    return 0;  // Command not found
}

// Basic commands
public int cmd_quit(string arg) {
    write("Goodbye!\n");
    logger->info("Player %s quit", name);
    destruct(this_object());
    return 1;
}

public int cmd_look(string arg) {
    write("You look around.\n");  // TODO: Implement proper room description
    return 1;
}

public int cmd_say(string arg) {
    if (!arg || arg == "") {
        write("Say what?\n");
        return 1;
    }
    write("You say: " + arg + "\n");
    return 1;
}

public int cmd_help(string arg) {
    write("Available commands: " + implode(m_indices(commands), ", ") + "\n");
    return 1;
}

public int cmd_who(string arg) {
    write("Players online: " + capitalize(name) + "\n");  // TODO: Implement proper player listing
    return 1;
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
