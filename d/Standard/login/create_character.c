/*
 * create_character.c
 * 
 * Streamlined character creation that assigns defaults without user interaction.
 * This replaces the multi-step process in bodies.c, features.c, mangle.c, and skills.c
 * 
 * INTEGRATION:
 * To use this streamlined process instead of the traditional multi-room character
 * creation, modify ghost_player.c's query_default_start_location() function to call:
 * 
 *   if (bypass_traditional_creation(this_object()) == 0)
 *       return query_def_start();  // Character created, go to starting location
 * 
 * USAGE:
 * - create_character(player) - Main entry point for default character creation
 * - quick_character_creation(player, race, gender) - With specific race/gender
 * - streamlined_ghost_creation(player) - For ghost players needing full creation
 * - test_character_creation() - Test function for wizards
 */

#pragma strict_types

#include <stdproperties.h>
#include "/conf/login/login.h"
#include <const.h>

private void assign_default_body(object player);
private void assign_default_physical_attributes(object player);
public mixed bypass_traditional_creation(object player);
public varargs void quick_character_creation(object player, string race, string gender);
private void assign_default_features(object player);
private void finalize_character(object player);

/*
 * Function name: create_character
 * Description  : Main entry point for character creation with default values
 * Arguments    : player - the player object to configure
 * Returns      : void
 */
public void create_character(object player)
{
    if (!objectp(player))
    {
        log_debug("create_character: Invalid player object");
        return;
    }

    // Set defaults for body creation
    assign_default_body(player);
    log_debug("Assigned default body for player: %O", player);
    
    // Set defaults for physical attributes (mangle step)
    assign_default_physical_attributes(player);
    log_debug("Assigned default physical attributes for player: %O", player);
    
    // Set defaults for features/adjectives
    assign_default_features(player);
    log_debug("Assigned default features for player: %O", player);
    
    // Finalize character creation
    finalize_character(player);
    log_debug("Character creation finalized for player: %O", player);
}

/*
 * Function name: assign_default_body
 * Description  : Assigns a default race and gender to the player
 * Arguments    : player - the player object
 * Returns      : void
 */
private void assign_default_body(object player)
{
    string race, gender_str;
    int gender, appearance;
    int *stats;
    
    // Choose random race from available races
    race = RACES[random(sizeof(RACES))];
    
    // Choose random gender
    gender = random(2);
    gender_str = gender ? "male" : "female";
    
    // Set basic character properties
    player->set_race_name(race);
    player->set_gender(gender ? G_MALE : G_FEMALE);
    
    // Set random appearance
    appearance = random(98) + 1;
    player->set_appearance(appearance);
    
    // Set race-specific stats
    if (RACESTAT[race])
    {
        stats = RACESTAT[race] + ({});  // Copy the array
        
        // Add some random variation (±1 to random stats)
        int i;
        for (i = 0; i < 3; i++)  // Add 3 random stat points
        {
            stats[random(6)]++;
        }
        
        player->set_stats(stats);
    }
    else
    {
        // Fallback default stats if race not found
        stats = ({ 9, 9, 9, 9, 9, 9 });
        // Add some random variation
        int i;
        for (i = 0; i < 3; i++)
        {
            stats[random(6)]++;
        }
        player->set_stats(stats);
    }
    
    log_debug("Assigned race: %s, gender: %s, stats: %O", race, gender_str, stats);
}

/*
 * Function name: assign_default_physical_attributes
 * Description  : Sets default height and weight based on race (mangle step)
 * Arguments    : player - the player object
 * Returns      : void
 */
private void assign_default_physical_attributes(object player)
{
    string race;
    int *attr;
    int height, weight;
    int height_modifier, weight_modifier;
    
    race = ({string}) player->query_race_name();
    
    if (!race || !RACEATTR[race])
    {
        log_debug("No race attributes found for race: %s", race);
        return;
    }
    
    attr = RACEATTR[race];
    
    // Use normal size (index 2 in SPREAD_PROC = 100%)
    height_modifier = SPREAD_PROC[2];
    weight_modifier = SPREAD_PROC[2];
    
    // Add slight random variation (±10%)
    height_modifier += random(21) - 10;  // -10 to +10
    weight_modifier += random(21) - 10;  // -10 to +10
    
    height = (attr[0] * height_modifier) / 100;
    weight = (attr[1] * weight_modifier) / 100;
    
    player->add_prop(CONT_I_HEIGHT, height);
    player->add_prop(CONT_I_WEIGHT, weight);
    
    log_debug("Assigned height: %d cm, weight: %d kg", height, weight);
}

/*
 * Function name: assign_default_features
 * Description  : Sets default adjectives/features for the player
 * Arguments    : player - the player object
 * Returns      : void
 */
private void assign_default_features(object player)
{
    string *default_adjectives;
    string race, gender_str;
    
    race = ({string}) player->query_race_name();
    gender_str = ({string}) player->query_gender_string();
    
    // Set basic adjectives: gender, race, and some default features
    default_adjectives = ({ 
        gender_str, 
        race, 
        "average",      // Default appearance adjective
        "ordinary"      // Default personality adjective
    });
    
    player->set_adj(default_adjectives);
    
    // Set the player as always known
    player->add_prop(LIVE_I_ALWAYSKNOWN, 1);
    
    log_debug("Assigned adjectives: %O", default_adjectives);
}

/*
 * Function name: finalize_character
 * Description  : Finalizes character creation and moves player to starting location
 * Arguments    : player - the player object
 * Returns      : void
 */
private void finalize_character(object player)
{
    string race, start_location;
    
    race = ({string}) player->query_race_name();
    
    // Clear ghost state
    player->set_ghost(0);
    
    // Call ghost_ready to complete the initialization
    player->ghost_ready();
    
    // // Get starting location for the race
    // if (RACESTART[race])
    // {
    //     start_location = RACESTART[race];
    // }
    // else
    // {
    //     start_location = ({string}) player->query_def_start();
    // }
    
    // // Move player to starting location
    // if (start_location)
    // {
    //     write("Character creation complete! Welcome to the world.\n");
    //     player->move_living("into the world", start_location);
    // }
    
    // log_debug("Character creation finalized for %O", player);
}

/*
 * Function name: quick_character_creation
 * Description  : Alternative entry point that can be called from login process
 * Arguments    : player - the player object
 *                race - optional specific race (defaults to random)
 *                gender - optional specific gender (defaults to random)
 * Returns      : void
 */
public varargs void quick_character_creation(object player, string race, string gender)
{
    if (!objectp(player))
    {
        return;
    }
    
    // If specific race/gender provided, set them before calling main creation
    if (race && member(RACES, race) != -1)
    {
        player->set_race_name(race);
    }
    
    if (gender)
    {
        int gender_val = (gender == "male") ? G_MALE : G_FEMALE;
        player->set_gender(gender_val);
    }
    
    create_character(player);
}

/*
 * Function name: streamlined_ghost_creation
 * Description  : Handles the complete character creation for ghost players
 *                bypassing the multi-room process
 * Arguments    : player - the ghost player object
 * Returns      : 1 if successful, 0 if failed
 */
public int streamlined_ghost_creation(object player)
{
    int ghost_state;
    
    if (!objectp(player))
    {
        return 0;
    }
    
    ghost_state = ({int}) player->query_ghost();
    
    // Only process if player needs character creation
    if (!(ghost_state & (GP_BODY | GP_MANGLE | GP_FEATURES | GP_SKILLS)))
    {
        log_debug("Player %s doesn't need character creation", ({string}) player->query_real_name());
        return 0;
    }
    
    write("Entering streamlined character creation...\n");
    
    // Perform all character creation steps at once
    create_character(player);
    
    write("Character creation completed successfully!\n");
    return 1;
}

/*
 * Function name: bypass_traditional_creation
 * Description  : Can be called to bypass the traditional multi-room creation process
 * Arguments    : player - the player object
 * Returns      : string path to move player to, or 0 to use streamlined creation
 */
public mixed bypass_traditional_creation(object player)
{
    int ghost_state;
    
    if (!objectp(player))
    {
        return "/d/Standard/login/bodies";  // Fallback to traditional
    }
    
    ghost_state = ({int}) player->query_ghost();
    
    // If player needs any creation steps, use streamlined process
    if (ghost_state & (GP_BODY | GP_MANGLE | GP_FEATURES | GP_SKILLS))
    {
        if (streamlined_ghost_creation(player))
        {
            return 0;  // Signal that streamlined creation was used
        }
    }
    
    // Fallback to traditional process
    return "/d/Standard/login/bodies";
}

/*
 * Function name: test_character_creation
 * Description  : Test function to demonstrate the streamlined character creation
 *                Can be called by wizards for testing
 * Arguments    : None (uses this_player())
 * Returns      : void
 */
public void test_character_creation()
{
    object player;
    
    player = this_player();
    
    if (!player)
    {
        write("No player found.\n");
        return;
    }
    
    if (!({int}) player->query_wiz_level())
    {
        write("This is a test function for wizards only.\n");
        return;
    }
    
    write("Testing streamlined character creation...\n");
    write("Current ghost state: " + ({int}) player->query_ghost() + "\n");
    write("Current race: " + ({string}) player->query_race_name() + "\n");
    write("Current adjectives: " + implode(({string *}) player->query_adj(1), ", ") + "\n");
    
    // Set player to need character creation for testing
    player->set_ghost(GP_NEW);
    
    write("\nRunning streamlined creation...\n");
    if (streamlined_ghost_creation(player))
    {
        write("Success! New character properties:\n");
        write("Race: " + ({string}) player->query_race_name() + "\n");
        write("Gender: " + ({string}) player->query_gender_string() + "\n");
        write("Adjectives: " + implode(({string *}) player->query_adj(1), ", ") + "\n");
        write("Height: " + ({int}) player->query_prop(CONT_I_HEIGHT) + " cm\n");
        write("Weight: " + ({int}) player->query_prop(CONT_I_WEIGHT) + " kg\n");
        write("Stats: " + implode(map(({int *}) player->query_stats(), #'to_string), ", ") + "\n");
        write("Ghost state: " + ({int}) player->query_ghost() + "\n");
    }
    else
    {
        write("Character creation failed.\n");
    }
}
