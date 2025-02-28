#pragma strict_types
#pragma save_types

#include "/inc/player.h"
#include "/inc/input_to.h"

private string name;          // Player's name
private int state;           // Current player state
private int level;          // Player level
private mapping commands;    // Available commands
private object logger;      // Logger instance

private void show_entrance();

public void create() {
    logger = load_object("/sys/log");
    commands = PLAYER_COMMANDS;  // Load basic commands
    state = PLAYER_STATE_LOADING;
}

public void initialize(string player_name) {
    name = player_name;
    state = PLAYER_STATE_PLAYING;
    logger->info("PLAYER", "Player %s initialized", name);
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
        object ob = this_object();
        
        // Call the command and ensure we get an int back
        result = (int)call_other(ob, command, args);
        if (intp(result)) return result;
        return 0;  // Non-integer results are treated as failure
    }
    
    return 0;  // Command not found
}

// Basic commands
public int cmd_quit(string arg) {
    write("Goodbye!\n");
    logger->info("PLAYER", "Player %s quit", name);
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
public string query_name() { return name; }
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
