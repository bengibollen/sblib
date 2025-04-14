#pragma strict_types
#pragma no_inherit

#include <config.h>
#include <input_to.h>  // For INPUT_* flags
#include <log.h>       // For logging
#include <libfiles.h>     // For PLAYER_OBJECT
#include <interactive_info.h>

#include <composite.h>
#include <const.h>
#include <language.h>
#include <login.h>
#include <macros.h>
#include <mail.h>
#include <ss_types.h>
#include <std.h>
#include <stdproperties.h>
#include <libtime.h>


// inherit "/lib/telnetneg.c";


// Constants
#define MAX_ATTEMPTS    3
#define IDLE_TIMEOUT    300
#define ATTEMPT_LOG  "/open/attempt"
#define GUEST_LOGIN  "guest"
#define CLEANUP_TIME 120.0 /* two minutes  */
#define TIMEOUT_TIME 120.0 /* two minutes  */
#define PASS_QUEUE   600   /* ten minutes */
#define PASS_ARMAGEDDON 300 /* 5 minutes*/
#define ONE_DAY      86400 /* one day in seconds */

#define ENTER_ENTER  0 /* notify that someone logged in              */
#define ENTER_REVIVE 3 /* notify that someone revived from linkdeath */
#define ENTER_SWITCH 4 /* notify that someone switched terminals     */


// Function prototypes
public void create();    // Must be public for object creation
public void logon();     // Must be public for driver calls
public void handle_name(string input);     // Changed to public for input_to
public void handle_password(string input);  // Changed to public for input_to
private void show_banner();
public void check_idle();
private void start_character_creation();
private int valid_name(string str);
private int user_exists(string name);
private int verify_password(string name, string pass);
private void login_success();
private void show_menu();
public void handle_menu(string input);
static void start_player();
public void new_player(string name);
public void new_password(string input);
private int check_password_rules(string password);

/*
 * Prototypes.
 */
static void check_password(string p);
static void tell_password();
static void try_throw_out(string str);
static void queue(string str);
static void waitfun(string str);
static void get_name(string str);

/*
 * Global valiables that aren't in the save-file.
 */
static int login_flag = 0; /* True if the player passed the queue.      */
static int login_type = ENTER_ENTER; /* Login/revive LD/switch terminal */
static int password_set = 0; /* New password set or not.                */
static string old_password; /* The old password of the player.          */

// Variables
private string name;
private int time_of_login;
private int login_attempts;

/*
 * These are the necessary variables stored in the save file.
 */
private string  password;        /* The password of the player         */
private string  player_file;     /* The racefile to use for the player */
private int     restricted;      /* Are we restricted?                 */


public void create() {
    log_info("=== Login Object Created ===");
    log_debug("Object name: %s", object_name(this_object()));
    time_of_login = time();
    call_out("check_idle", IDLE_TIMEOUT);
}


public void logon() {
    log_info("New login session started");
    log_debug("Object name: %s", object_name(this_object()));

    write("\nWelcome to SBLib MUD!\n");
    show_banner();
    show_menu();
}


public void check_idle() {
    log_info("Checking idle status - Timeout occurred");
    write("\nTimeout - disconnecting.\n");
    destruct(this_object());
}


private void show_banner() {
    write("\n=== Welcome to SBLib MUD ===\n");
    write("Your journey begins here...\n\n");
}


private void show_menu() {
    write("Please select an option:\n");
    write("1. Login to existing character\n");
    write("2. Create new character\n");
    write("3. Exit\n");
    write("\nYour choice (1-3): ");
    input_to(#'handle_menu);
}


public void handle_menu(string input) {
    log_info("=== Handling Menu Input ===\n");
    switch(input) {
        case "1":
            write("Name: ");
            input_to(#'handle_name);
            break;
        case "2":
            write("Creating new character...\n");
            write("Please enter your character's name: ");
            input_to(#'new_player);
            break;
        case "3":
            write("\nGoodbye! Come back soon!\n");
            destruct(this_object());
            break;
        default:
            write("\nInvalid choice.\n\n");
            show_menu();
            break;
    }
}


public void new_player_entry()
{
    object player_obj;
    log_info("=== New Player Entry ===\n");
    log_debug("Object name: %s", object_name(this_object()));
    write("\nWelcome to SBLib MUD!\n");
    write("You are now entering the world of Silver Bucket.\n");
    write("Please wait while we set up your character...\n");
    player_obj = clone_object(LOGIN_NEW_PLAYER);
    player_obj->open_player();

    configure_object(player_obj, OC_EUID, BACKBONE_UID);

    player_obj->set_trusted(1);
    exec(player_obj, this_object());
    player_obj->enter_new_player(name, password);
    destruct(this_object());
    return;
}


public void handle_intro(string input)
{
    
    log_info("=== Handling Intro Input ===\n");
    log_debug("Input: %s", input);
    switch(input) {
        case "y":
            write("Welcome to your new adventure!\n");
            break;
        case "n":
            write("You have chosen to skip the intro.\n");
            new_player_entry();
            
            break;
        case "q":
            write("Goodbye! Come back soon!\n");
            destruct(this_object());
            break;
        default:
            write("Invalid choice. Please enter y[es], n[o], or q[uit]: ");
            input_to(#'handle_intro);
            break;
    }
}


private void intro()
{
    write("You wake up from the darkness...\n");
    write("How long have you been asleep?\n");
    write("You open your eyes and see nothing but bright light.\n");
    write("You feel a sense of confusion as you try to remember your past.\n");
    write("How did you get here?\n");
    write("Who are you?\n");
    write("Where were you before you got here?\n");
    write("What are you?\n");
    write("- I am...\n");
    write("- I am ... " + capitalize(name) + "\n");
    write("- ... " + capitalize(name) + "... ?\n");
    write("How did you know that?\n");
    write("You look down at your hands but see nothing but a translucent blur.\n");
    write("Suddenly you hear an overbearing voice in your head.\n");
    write("- So... You have finally arrived, the voice booms.\n");
    write("You think you hear papers shuffling.\n");
    write("- I have been waiting for you...erm..\n");
    write("More papers shuffling.\n");
    write("- " + capitalize(name) + "?\n");
    write("You nod silently, or at least you think you do.\n");
    write("- Oh, sorry. I'll release you.\n");
    write("You hear the snapping of fingers.\n");
    write("You feel a sudden rush of clarity as the world around you comes into focus.\n");
    write("Your muddled senses clear and you can see you are standing on a flat surface that seems to stretch endlessly before you.\n");
    write("Above you is a vast expanse of blue sky, without a cloud in sight.\n");
    write("In front of you sits a scrawny old man at a desk, sifting through a pile of papers.\n");
    write("He looks up at you and smiles.\n");
    write("- Better now? he says in a crackling voice.\n");
    write("You nod again, this time more confidently.\n");
    write("- Good, good. No need to be afraid. Have a seat.\n");
    write("He says, pointing at a chair in front of the desk.\n");
    write("- I am god... or A god would be more correct.\n");
    write("- Your soul has been assigned for relocation.\n");
    write("He says, looking at the papers on his desk.\n");
    write("- As you might have guessed, you are dead.\n");
    write("He says, looking at you with a twinkle in his eye.\n");
    write("- But don't worry, you will be assigned a new body.\n");
    write("- You will not be reborn, per se, but you will still be able to explore your new existence.\n");
    write("He looks down on the papers again.\n");
    write("- Your new world is for some reason called Silver Bucket.\n");
    write("- I don't know why, but it seems to be a popular name.\n");
    write("- The rules of this world are a bit different from the one you come from.\n");
    write("- As if you could remember anything from your previous life, but still...\n");
    write("- Some concepts from your previous life still reside in your soul.\n");
    write("- Now to the important part.\n");
    write("- You have to choose a new body.\n");
    write("- You can choose between a human, a human, a human or a human.\n");
    write("He adjusts his glasses and looks at the paper in his hand.\n");
    write("- I am not sure why, but it seems to be the only option.\n");
    write("- I guess you will have to make do with what you have.\n");
    write("- The world is still in early access after all.\n");
    write("- There should be a choice of classes available too...\n");
    write("- Buuut.... Early access.\n");
    write("- You might encounter some bugs along the way.\n");
    write("- But don't worry, that's part of the adventure!\n");
    write("- I guess that's it for now.\n");
    write("- You will be able to acquire the new body from the church.\n");
    write("- I will send you on your way now, No reason to keep you here.\n");
    write("- Have fun!\n");
    write("He snaps his fingers again and you feel a sudden rush of energy.\n");
    write("You feel your body being pulled in all directions at once as you brace yourself.\n");
    write("A sudden rush of energy flows through your body as you are pulled into the new world.\n");
    write("ZZZZZzzzzzzzappppppp!\n");
    write(read_file(LOGIN_FILE_WELCOME));
}


private void create_player()
{
    // object player_obj;

    // player_obj = clone_object(LOGIN_NEW_PLAYER);
    if (1) //(objectp(player_obj))
    {
        // exec(player_obj, this_object());
        write("Do you want the newbie intro?\n");
        write("y[es], n[o] or q[uit]? ");
        input_to(#'handle_intro);
    }
    else
    {
        write("Failed to create player object.\n");
        destruct(this_object());
    }

}


public void confirm_password(string input) {
    log_info("=== Confirming Password ===\n");
    log_debug("Input: %s", input);

    if (input == password)
    {
        write("\nCreating new character...\n");
        cat(LOGIN_FILE_NEW_PLAYER_INFO);
        input_to(#'create_player, INPUT_PROMPT, "Press ENTER to continue...");
    } 
    else 
    {
        write("\nPasswords do not match. Please try again: ");
        password = "";
        input_to(#'new_password, INPUT_NOECHO);
    }
}


public void new_password(string input) {
    log_info("=== New Password ===\n");
    log_debug("Input: %s", input);

    if (check_password_rules(input))
    {
        password = input;
        write("\nPlease confirm your password: ");
        input_to(#'confirm_password, INPUT_NOECHO);
    } 
    else 
    {
        write("\nInvalid password. Please try again: ");
        input_to(#'new_password, INPUT_NOECHO);
    }
} 

public void confirm_name(string input) {
    log_info("=== Confirming Name ===\n");
    log_debug("Input: %s", input);
    input = lower_case(input);

    if (input[0] == 'y')
    {
        log_debug("Confirmed name: %s", name);
        write("\nPlease enter your password: ");
        input_to(#'new_password, INPUT_NOECHO);
    }
    else if (input[0] == 'n')
    {
        name = "";
        log_debug("User chose to enter a new name.");
        write("\nPlease enter your character's name: ");
        input_to(#'new_player);
    }
    else if (input[0] == 'q')
    {
        name="";
        write("\nGoodbye! Come back soon!\n");
        destruct(this_object());
    }
    else
    {
        write("\nInvalid choice. Please enter y[es], n[o], or q[uit]: ");
        input_to(#'confirm_name);
    }
}

public void new_player(string input)
{
    log_info("=== Creating New Player ===\n");
    log_debug("New player name: %s", input);
    if (!valid_name(input)) {
        log_debug("Invalid name: %s", input);
        write("Invalid name. Please choose a name between 3 and 16 characters, using only lowercase letters.\n");
        write("Please enter your character's name: ");
        input_to(#'new_player);
        return;
    }
    
    if (user_exists(input)) {
        log_debug("User already exists: %s", input);
        write("User already exists. Try a different name.\n");
        write("Please enter your character's name: ");
        input_to(#'new_player);
        return;
    }
    name = input;

    write("Do you really want to use the name " + capitalize(name) +
    "? y[es], n[o] or q[uit]? ");
    input_to(#'confirm_name);

//    input_to(#'confirm_name);

    // password = "";
    // write("Please enter your password: ");
    // input_to("new_password", INPUT_NOECHO);
}


public void handle_name(string name)
{
    log_info("=== Handling Name Input ===\n");
    log_debug("Object name: %s", object_name(this_object()));
    log_debug("Processing name input: %s", name);
   
    name = lower_case(name);
    
    log_debug("Player file: %s", PLAYER_FILE(name));

    if (restore_object(PLAYER_FILE(name)))
    {
        write("\nPassword: ");
        input_to(#'handle_password, INPUT_NOECHO);    
    }
    else
    {
        write("User not found. Try 'new' to create a character.\n");
        show_menu();
        return;
    }
}


public void handle_password(string pass)
{
    if (++login_attempts >= MAX_ATTEMPTS) {
        write("\nToo many failed attempts. Disconnecting.\n");
        destruct(this_object());
        return;
    }
    
    if (crypt(pass, password) != password)
    {
        write("\nIncorrect password. Try again: ");
        input_to(#'handle_password, INPUT_NOECHO);
        return;
    }

    start_player();
}


private void login_success() {
    object player;
    
    log_info("Successful login for user: %s", name);
    log_debug("Object name: %s", object_name(this_object()));
    log_debug("This player object name: %s", object_name(this_player()));
    log_debug("This interactive object name: %s", object_name(this_interactive()));

    write("\nWelcome back, " + capitalize(name) + "!\n");
    player = clone_object(PLAYER_OBJECT);
    log_info("Cloning player object: %s", object_name(player));

    player->initialize(name);
    if (!player)
    {
        write("Sorry, there was an error during login.\n");
        destruct(this_object());
        return;
    }

    log_info("Successfully cloned player object: %s", object_name(player));
    
    int success = exec(player, this_object());
    log_debug("Exec result: %d", success);

    if (!success)
    {
        log_error("Failed to exec player object");
        destruct(player);
        destruct(this_object());
        return;
    }
    log_info("Player object exec'd successfully");
    player->move("w/debug/workroom");
    player->player_startup();
    log_debug("Player object startup completed");


    log_info("Player moved to: w/debug/workroom");

    destruct(this_object());
}


// Utility functions
private int valid_name(string str)
{
    log_info("Validating name: %s", str);
    if (stringp(str))
    {
        log_info("Name is a string: %s", str);
        if (sizeof(str) >= 3 && sizeof(str) <= 16)
        {
            log_info("Name length is valid: %d", sizeof(str));
            if (!!regmatch(str, "^[a-z]+$"))
            {
                log_info("Name does not contain invalid characters: %s", str);
                return 1;
            }
        }
    }
    log_info("Name is not a valid string: %s", str);
    return 0;
}


private int user_exists(string name)
{
    log_debug("Checking user existence for: %s", name);
    log_debug("Player file: %s", PLAYER_FILE(name) + ".o");
    log_debug("File size: %d", file_size(PLAYER_FILE(name)));
    // TODO: Implement user checking
    return file_size(PLAYER_FILE(name) + ".o") > 0;
}


private int check_password_rules(string password)
{
    return 1;

    if (sizeof(password) < 8) {
        write("Password must be at least 8 characters long.\n");
        return 0;
    }


    if (!regmatch(password, "[A-Z]")) {
        write("Password must contain at least one uppercase letter.\n");
        return 0;
    }


    if (!regmatch(password, "[a-z]")) {
        write("Password must contain at least one lowercase letter.\n");
        return 0;
    }


    if (!regmatch(password, "[0-9]")) {
        write("Password must contain at least one digit.\n");
        return 0;
    }


    if (!regmatch(password, "[^a-zA-Z0-9]")) {
        write("Password must contain at least one special character.\n");
        return 0;
    }

    return 1;
}


private int verify_password(string name, string pass) {
    // TODO: Implement password verification
    write("Verifying password for user: " + name + "\n");
    return 1;
}


private void start_character_creation() {
    write("\nCharacter creation not implemented yet.\n");
    show_menu();
}






/*
 * Function name: create_object
 * Description  : Called to construct this object.
 */
static void creates()
{
}


/*
 * Function name: short
 * Description  : This function returns the short description of this object.
 * Returns      : string - the short description.
 */
string short()
{
    return "login"  + (name ? " (" + name + ")" : "");
}


/*
 * Function name: query_pl_name
 * Description  : Return the real name of the player who is trying to log in.
 * Returns      : string - the name.
 */
string query_pl_name()
{
    return name;
}


/*
 * Function name: query_real_name
 * Description  : Return the real name of this object: "logon"
 * Returns      : string - "logon".
 */
string query_real_name()
{
    return "logon";
}


/*
 * Function name: start_player2
 * Description  : Swapsocket to player object and if we are not already
 *                in the game enter it.
 * Arguments    : object ob - the playerobject to swap to.
 */
static void start_player2(object ob)
{
    object dump;
    int old_was_live = 0;

    /* Print possible news to the player before we alter his/her euid.
     * Since cat() doesn't seem to work, even when setting this_player to
     * this_object, we have to use this construct to make sure the person
     * gets to read the message.
     */
    write(read_file(LOGIN_FILE_NEWS));

    /* If the old socket was already interactive, we must swap them
     * nicely. First tell them what is happening, than clone a new
     * object, swap them out and destruct the old one. We use the
     * LOGIN_NEW_PLAYER since that doesn't leave a 'notify' message when
     * destructed.
     */
    if (interactive(ob))
    {
        if (environment(ob))
        {
            tell_room(environment(ob), ({
                capitalize(({string}) ob->query_real_name()) + " renews " +
                ({string}) ob->query_possessive() + " contact with reality.\n",
                "The " + ({string}) ob->query_nonmet_name() + " renews " +
                    ({string}) ob->query_possessive() + " contact with reality.\n",
                "" }),
                ({ ob }) );
        }

        tell_object(ob,
            "New interactive link to your body. Closing this connection.\n");

        dump = clone_object(LOGIN_NEW_PLAYER);
        /* Swap old socket to dummy player. */
        exec(dump, ob);
        dump->remove_object();
        old_was_live = 1;
    }

    
    log_debug("Swapping to the player object.");
    log_debug("This object: " + to_string(this_object()) + "\n");
    log_debug("Ob object: " + to_string(ob) + "\n");

    /* Swap to the playerobject. */
    exec(ob, this_object());

    /* If we are not in the game, enter it. */
    if (!environment(ob))
    {
        if (!(({int}) ob->enter_game(name, (password_set ? password : ""))))
        {
            write("Illegal playerfile.\n");
            ob->remove_object();
        }
    }
    else if (!old_was_live)
    {
        ob->revive();
        ob->fixup_screen();
    }
    else
    {
        ob->fixup_screen();
    }

    /* Notify the wizards of the action. */
    SECURITY->notify(ob, login_type);

    ob->update_hooks();
    destruct(this_object());
}


/*
 * Function name: start_player1
 * Description  : The next step in the startup process.
 */
static void start_player1()
{
    object ob;
    
    log_debug("Running start player 1");

    /* Now we can enter the game, find the player file */
    if (player_file)
    {
        log_debug("Attempting to clone player file.");
        ob = clone_object(player_file);

        if (function_exists("enter_game", ob) != PLAYER_SEC_OBJECT)
        {
            log_debug("Player file does not have the required function.");
            ob->remove_object();
        }

        if (!objectp(ob))
        {
            write("Your body cannot be found.\n" +
                "Therefore you must choose a new.\n\n");
            player_file = 0;
            show_menu();
            return;
        }
    }

    /*
     * There can be three different reasons for not having a player_file:
     *
     *    1 - If this is a new character, let the login player object
     *        manage the creation / conversion / process.
     *    2 - The players racefile is not loadable, a new body must be
     *        choosen.
     *    3 - The players racefile is not a legal playerfile, a new body
     *        must be choosen.
     */
    if (!player_file ||
        (player_file == LOGIN_NEW_PLAYER))
    {
        /* Only clone if we have not done so yet. */
        if (!objectp(ob))
        {
            if (name[..<2] == "sr")
            {
                write("\nCreating test character player file.\n");
                ob = clone_object(LOGIN_TEST_PLAYER);
            }
            else
                ob = clone_object(LOGIN_NEW_PLAYER);
        }
        ob->open_player();

        configure_object(ob, OC_EUID, BACKBONE_UID);

        ob->set_trusted(1);
        exec(ob, this_object());
        ob->enter_new_player(name, password);
        destruct(this_object());
        return;
    }

    ob->open_player();

    if (({int}) SECURITY->query_wiz_rank(name))
        configure_object(ob, OC_EUID, name);
    else
        configure_object(ob, OC_EUID, BACKBONE_UID);

    ob->set_trusted(1);
    start_player2(ob);
}


/*
 * Function name: start_player
 * Description  : This function checks for linkdeath and sees whether the
 *                player has to queue. If there are no restrictions, log in
 *                immediately.
 */
static void start_player()
{
    object other_copy;
    int    pos;

    /* If there is no other copy of the player in the game, we can try to
     * log in immediately if the player doesn't have to queue.
     */
    other_copy = find_player(name);

    if (!objectp(other_copy))
    {
        log_debug("No other copy found, starting player.");
        start_player1();
        return;
    }

    /* When 'login_flag' is true, this means the player already queued (after
     * having been linkdead. Reconnect instantly.
     */
    if (login_flag)
    {
        login_type = ENTER_REVIVE;
        start_player2(other_copy);
        return;
    }

    /* If you already have a link, you are asked to switch terminals */
    if (interactive(other_copy))
    {
        write("You are already playing !\n");
        return;
    }

    /* The player is linkdead, but in combat, reconnect immediately. */
    if (({int}) other_copy->query_linkdead_in_combat())
    {
        write("You were in combat when your link broke.\n" +
            "... instantly reconnecting ...\n\n");
        login_type = ENTER_REVIVE;
        start_player2(other_copy);
        return;
    }

    /* Player was linkdead for less PASS_QUEUE seconds. */
    if ((time() - ({int}) other_copy->query_linkdead()) < PASS_QUEUE)
    {
        write("You were linkdead less than ten minutes ...\n" +
            "... instantly connecting ...\n\n");
        login_type = ENTER_REVIVE;
        start_player2(other_copy);
        return;
    }

    write("You have been linkdead for " +
        CONVTIME(time() - ({int}) other_copy->query_linkdead()) + ".\n");

    login_type = ENTER_REVIVE;
    start_player2(other_copy);
    return;
}


/*
 * Function name: confirm_use_name
 * Description  : When a player first connects, we give him a little message
 *                about the user of proper names, and then ask him to confirm
 *                the use of the name.
 * Arguments    : string str - the entered text.
 */
static void confirm_use_name(string input)
{
    /* Only allow valid answers. */
    input = lower_case(input);
    if (input[0] == 'q')
    {
        write("\nWelcome another time then!\n");
        destruct(this_object());
        return;
    }


    if (input[0] == 'n')
    {
        write("\nThen please select a different name, or use 'quit' " +
            "to disconnect.\n\nPlease enter your name: ");
        input_to(#'get_name);
        return;
    }

    if (input[0] != 'y')
    {
        write("\nPlease answer with either y[es], n[o] or q[uit].\n" +
            "Would you really like to use the name " + capitalize(input) + "? ");
        input_to(#'confirm_use_name);
        return;
    }

    write("\nWelcome, " + capitalize(name) +
        ". Please enter your password.\n\n");
    tell_password();
}


/*
 * Function name: get_name
 * Description  : At login time, this function is called with the name the
 *                player intends to use. Some checks are made and when it
 *                is all correct, the player may login.
 * Arguments    : string str - the name the player wants to use.
 */
static void get_name(string str)
{
    object g_info;
    object a_player;
    int i;
    int runlevel;
    int delay;
    int vowels;


    str = lower_case(str);
    if (str == "quit")
    {
        write("\nWelcome another time then!\n");
        destruct(this_object());
        return;
    }

    if (!valid_name(str))
    {
        input_to(#'get_name);
        write("Give name again: ");
        return;
    }

    /* If the runlevel is set, the not all players may enter. */
    if (runlevel = ({int}) SECURITY->query_runlevel())
    {
#ifdef ATTEMPT_LOG
        write_file(ATTEMPT_LOG, ctime(time()) + " " + capitalize(str) + "\n");
#endif

        switch(runlevel)
        {
        case WIZ_APPRENTICE:
            write("\nThe game is currently open for wizards only.\n");
            break;

        case WIZ_ARCH:
            write("\nThe game is currently open for members of the " +
                "administration.\n");
            break;

        default:
            write("\nThe game is currently only open for wizards of the rank " +
                WIZ_RANK_NAME(runlevel) + " and higher.\n");
        }

        /* Player is not allowed in, but do allow juniors. */
        if ((({int}) SECURITY->query_wiz_rank(str) < runlevel) &&
            !(str[..<2] == "jr" &&
              (({int}) SECURITY->query_wiz_rank(str[..<3]) >= runlevel)))
        {
            if (file_size(LOGIN_NO_NEW) > 0)
            {
                cat(LOGIN_NO_NEW);
            }

            destruct(this_object());
            return;
        }
    }

    /* When Armageddon is active, players may not be allowed to connect. */
    // if (({int}) ARMAGEDDON->shutdown_active())
    // {
    //     delay = ({int}) ARMAGEDDON->query_delay();

    //     /* But 'full' wizards (++) are always allowed access. */
    //     if (({int}) SECURITY->query_wiz_rank(str) >= WIZ_NORMAL)
    //     {
    //         write("\nArmageddon is active, but you can login anyway.\n");
    //         write("Shutdown in " + CONVTIME(delay) + ".\n");
    //     }
    //     else if (delay > PASS_ARMAGEDDON)
    //     {
    //         write("\nArmageddon is active, but you can still login.\n");
    //         write("Shutdown in " + CONVTIME(delay) + ".\n");
    //     }
    //     else
    //     {
    //         write("\nArmageddon is active, The game is close to a " +
    //             "reboot. Please try again when\nthe game is back up.\n\n");
    //         write("Shutdown in " + CONVTIME(delay) + ".\n\n");
    //         write("NOTE: After the game shut down, it may take a few " +
    //             "minutes before the game\nis up and accessible again.\n");

    //         destruct(this_object());
    //         return;
    //     }
    // }

    /* Restore the player. If that fails, we make some additional checks
     * for we must be dealing with a new player.
     */
    if (!restore_object("/players/" + str))
    {
        log_debug("Failed to restore player: %s", str);

        vowels = sizeof(filter(explode(str, ""), (: $1 in LANG_VOWELS :)));
        if (!vowels)
        {
            write("\nYour name must contain at least one vowel. " +
                "(i.e. \"aeiouy\".)\n");
            input_to(#'get_name);
            write("Give another name: ");
            return;
        }
        if (vowels == sizeof(str))
        {
            write("\nYour name must contain at least one consonant. " +
                "(i.e. other than \"aeiouy\").\n");
            input_to(#'get_name);
            write("Give another name: ");
            return;
        }


        /* The new player is an old wizard, that is not removed correctly. */
        if (({int}) SECURITY->query_wiz_rank(name))
        {
            write("\nThis name used to belong to a wizard, but has " +
                "not been freed in a correct manner. If you used to have a " +
                "character here with this name or if you want to use the " +
                "name, you should contact the administration.\n");
            write("You can use guest-login to contact the " +
                "administration.\n");
            write("Give name again: ");
            input_to(#'get_name);
            return;
        }

        write("\nNew character.\n");
        cat(LOGIN_FILE_NEW_PLAYER_INFO);
        write("Do you really want to use the name " + capitalize(str) +
            "? y[es], n[o] or q[uit]? ");
        player_file = 0;
        name = str;
        input_to(#'confirm_use_name);
        return;
    }

    if (name == GUEST_LOGIN)
    {

        write("\nWelcome, guest. You do not need a password....\n" +
            "... connecting ...\n");

        start_player();
        return;
    }

    if (player_file)
    {
        write("\nWelcome, " + capitalize(name) +
            ". Please enter your password: ");
        input_to(#'check_password, 1);
    }
    else
    {
        write("\nWelcome, " + capitalize(name) +
            ". Please enter your password.\n");
        tell_password();
    }
}


/*
 * Function name: check_password
 * Description  : If an existing player tries to login, this function checks
 *                for the password. If you fail, you are a granted a second
 *                try.
 * Arguments    : string p - the intended password.
 */
static void check_password(string p)
{
    object *players;
    int     size;
    int     index;
    object  player;
    string *names;

    write("\n");


    /* Player has no password, force him/her to set a new one. */
    if (password == 0)
    {

        write("You have no password!\n" +
            "Set a password before you are allowed to continue.\n\n");
        password_set = 1;
        old_password = password;
        password = 0;
        tell_password();
        return;
    }

    /* Password doesn't match */
    if (crypt(p, password) != password)
    {
        write("Wrong password!\n");

        /* Player already had a second chance. Kick him/her out. */
        if (login_flag)
        {
            destruct(this_object());
            return;
        }

        login_flag = 1;
        write("Password (second and last try): ");
        input_to(#'check_password, 1);
        return;
    }


    /* Reset the login flag so people won't skip the queue. */
    login_flag = 0;

    start_player();
    return;
}


/*
 * Function name: query_race_name
 * Description  : Return the race name of this object.
 * Returns      : string - "logon".
 */
public string query_race_name()
{
    return "logon";
}


/*
 * Function name: catch_tell
 * Description  : This function can be called externally to print a text to
 *                the logon-player.
 * Arugments    : string msg - the text to print.
 */
public void catch_tell(string msg)
{
    write(msg);
}


/*
 * Function name: query_login_flag
 * Description  : Returns the current login flag.
 * Returns      : int - the login flag.
 */
public int query_login_flag()
{
    return login_flag;
}


/*
 * Function name: query_prevent_shadow
 * Description  : This function prevents shadowing of this object.
 * Returns      : int 1 - always.
 */
nomask public int query_prevent_shadow()
{
    return 1;
}


/*
 * Function name: login
 * Description  : This function is called when a player wants to login.
 *                A lot of checks are made.
 * Returns      : int 1/0 - true if login is allowed.
 */
public int logon1()
{
//  set_screen_width(80);

    if (!interactive(this_object()))
    {
        destruct(this_object());
        return 0;
    }

    /* No players from this site whatsoever. */
    if (({int}) SECURITY->check_newplayer(interactive_info(this_object(), II_IP_NUMBER)) == 1)
    {
        write("\nYour site is blocked due to repeated offensive " +
            "behaviour by users from your site.\n\n");
        destruct(this_object());
        return 0;
    }

    player_file = 0;

    configure_object(this_object(), OC_EUID, getuid());
    cat(LOGIN_FILE_WELCOME);

    write("Gamedriver version:  " + ({string}) SECURITY->do_debug("version") +
        "\nMudlib version    :  " + MUDLIB_VERSION +
        "\n\nPlease enter your name: ");


    input_to(#'get_name);

    return 1;
}


/*
 * Function name: new_password
 * Description  : This function is used to let a new character set his
 *                password.
 * Arguments    : string p - the intended password.
 */
static void new_passwordo(string p)
{
    write("\n");

    /* If the player does not want to use this character, he can type "quit"
     * as password.
     */
    if (p == "quit")
    {
        write("Very well. Until another time, perhaps.\n");
        destruct(this_object());
        return;
    }


    /* Player decided to enter a different name. */
    if (p == "new")
    {
        write("Please enter your name: ");
        input_to(#'get_name);
        return;
    }
    if (sizeof(p) < 6)
    {
        write("The password must have at least 6 characters.\n");
        input_to(#'new_password, 1);
        write("Password: ");
        return;
    }

    if (!(({int}) SECURITY->proper_password(p)))
    {
        write("The password does not match the basic security " +
            "standards we have set.\n");
        input_to(#'new_password, 1);
        write("Password: ");
        return;
    }

    if (sizeof(old_password) &&
        (crypt(p, old_password) == old_password))
    {
        write("The password must differ from the previous password.\n");
        write("Password: ");
        input_to(#'new_password, 1);
        return;
    }

    if (password == 0)
    {
        password = p;
        input_to(#'new_password, 1);
        write("Now please type the password again to verify.\n");
        write("Password (again): ");
        return;
    }

    if (password != p)
    {
        password = 0;
        write("The passwords don't match. You shall have to be " +
            "consistent next time!\n");
        input_to(#'new_password, 1);
        write("Password (new password, first try): ");
        return;
    }

    /* Crypt the password. Use a new seed. */
    password = crypt(password, 0);

    if (password_set)
    {
        start_player();
    }
    else
    {
        start_player1();
    }
}


/*
 * Function name: tell_password
 * Description  : This function tells the player what we expect from his
 *                new password and then prompt him for it.
 */
static void tell_password()
{
    write("To prevent people from breaking your password, we feel " +
        "the need to\nrequire your password to match certain criteria:\n" +
        "- the password must be at least 6 characters long;\n- the password " +
        "must at least contain one 'special character';\n- a 'special " +
        "character' is anything other than a-z and A-Z;\n- the 'special " +
        "character' may not be the first or the last\n  letter in the " +
        "password, that is somewhere before and after a\n  'special " +
        "character' there must be a normal letter.\n\nNew password: ");
    input_to(#'new_password, 1);
}


/*
 * Function name: valid_name
 * Description  : Check that a player name is valid. The name must be at
 *                least two characters long and at most eleven characters.
 *                We only allow lowercase letters. Also, generally offensive
 *                names are not allowed.
 * Arguments    : string str - the name to check.
 * Returns      : int 1/0 - true if the name is allowed.
 */
int ovalid_name(string str)
{
    int index = -1;
    int length = sizeof(str);

    if (length < 2)
    {
        write("\nThe name is too short. The minimum is 2 characters.\n");
        return 0;
    }


    while (++index < length)
    {
        if ((str[index] < 'a') ||
            (str[index] > 'z'))
        {
            write("\nInvalid characters in name \"" + str + "\".\n");
            write("Only letters (a through z) are allowed.\n");
            write("Character number was " + (index + 1) + ".\n");
            return 0;
        }
    }

    return 1;
}
