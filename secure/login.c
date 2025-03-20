#pragma strict_types
#pragma no_inherit

#include <config.h>
#include <input_to.h>  // For INPUT_* flags
#include <log.h>       // For logging
#include <libfiles.h>     // For PLAYER_OBJECT

// inherit "/lib/telnetneg.c";


// Constants
#define MAX_ATTEMPTS    3
#define IDLE_TIMEOUT    300
#define STATE_INIT      0
#define STATE_GET_NAME  1
#define STATE_NEW_CHAR  2
#define STATE_PASSWORD  3

// Function prototypes
public void create();    // Must be public for object creation
public void logon();     // Must be public for driver calls
public void handle_name(string input);     // Changed to public for input_to
public void handle_password(string input);  // Changed to public for input_to
private void show_banner();
private void prompt_name();
public void check_idle();
private void start_character_creation();
private int valid_name(string str);
private int user_exists(string name);
private int verify_password(string name, string pass);
private void prompt_password();
private void login_success();

// Variables
private string name;
private int time_of_login;
private int login_attempts;
private int current_state;
private object logger = load_object(LOG_FILE);

public void create() {
    logger->info("=== Login Object Created ===");
    logger->debug("Object name: %s", object_name(this_object()));
    time_of_login = time();
    current_state = STATE_INIT;
    call_out("check_idle", IDLE_TIMEOUT);
}

public void logon() {
    logger->info("New login session started");
    logger->debug("Object name: %s", object_name(this_object()));

    write("\nWelcome to SBLib MUD!\n");
    show_banner();
    prompt_name();
}

public void check_idle() {
    write("\nTimeout - disconnecting.\n");
    destruct(this_object());
}

private void show_banner() {
    write("Enter 'new' to create a character.\n");
    write("Enter 'quit' to disconnect.\n\n");
}

private void prompt_name() {
    current_state = STATE_GET_NAME;
    write("Name: ");
    input_to("handle_name");
}

private void prompt_password() {
    current_state = STATE_PASSWORD;
    write("\nPassword: ");
    input_to("handle_password", INPUT_NOECHO);  // Use proper flag
}

public void handle_name(string input) {
    logger->info("=== Handling Name Input ===\n");
    logger->debug("Object name: %s", object_name(this_object()));
    logger->debug("Processing name input: %s", input);
    if (!input || input == "") {
        write("Invalid name. Try again: ");
        input_to("handle_name");
        return;
    }
    
    input = lower_case(input);
    
    if (input == "quit") {
        write("Goodbye!\n");
        // if (this_player()) {
        //     this_player()->quit();  // Proper cleanup
        // }
        destruct(this_object());
        return 0;
    }
    
    if (input == "new") {
        current_state = STATE_NEW_CHAR;
        start_character_creation();
        return;
    }

    if (!valid_name(input)) {
        write("Invalid name. Try again: ");
        input_to("handle_name");
        return;
    }
    
    name = input;
    if (user_exists(name)) {
        prompt_password();
    }
    else {
        write("User not found. Try 'new' to create a character.\n");
        prompt_name();
    }
}

public void handle_password(string input) {
    if (++login_attempts >= MAX_ATTEMPTS) {
        write("\nToo many failed attempts. Disconnecting.\n");
        destruct(this_object());
        return;
    }
    
    if (verify_password(name, input)) {
        login_success();
    }
    else {
        write("\nIncorrect password. Try again: ");
        input_to("handle_password", INPUT_NOECHO);
    }
}

private void login_success() {
    object player;
    
    logger->info("Successful login for user: %s", name);
    logger->debug("Object name: %s", object_name(this_object()));
    logger->debug("This player object name: %s", object_name(this_player()));
    logger->debug("This interactive object name: %s", object_name(this_interactive()));

    write("\nWelcome back, " + capitalize(name) + "!\n");
    player = clone_object(PLAYER_OBJECT);
    logger->info("Cloning player object: %s", object_name(player));
    player->initialize(name);
    if (!player)
    {
        write("Sorry, there was an error during login.\n");
        destruct(this_object());
        return;
    }

    logger->info("Successfully cloned player object: %s", object_name(player));
    
    int success = exec(player, this_object());
    logger->debug("Exec result: %d", success);

    if (!success)
    {
        logger->error("Failed to exec player object");
        destruct(player);
        destruct(this_object());
        return;
    }
    logger->info("Player object exec'd successfully");
    player->player_startup();
    logger->debug("Player object startup completed");


    destruct(this_object());
}

// Utility functions
private int valid_name(string str) {
    return stringp(str) 
           && sizeof(str) >= 3 
           && sizeof(str) <= 16
           && !!regmatch(str, "^[a-z]+$"); // !! converts to 0 or 1
}

private int user_exists(string name) {
    // TODO: Implement user checking
    return 1;
}

private int verify_password(string name, string pass) {
    // TODO: Implement password verification
    return 1;
}

private void start_character_creation() {
    write("\nCharacter creation not implemented yet.\n");
    prompt_name();
}
