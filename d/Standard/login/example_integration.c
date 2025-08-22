/*
 * example_integration.c
 * 
 * Example showing how to integrate the streamlined character creation
 * into the existing login system.
 */

// Example modification for ghost_player.c query_default_start_location():

/*
public string query_default_start_location()
{
    mixed result;
    
    // Use streamlined character creation instead of multi-room process
    result = "/d/Standard/login/create_character"->bypass_traditional_creation(this_object());
    
    if (result == 0)
    {
        // Streamlined creation was successful, go to starting location
        if (query_ghost() == 0)
            return RACESTART[this_object()->query_race_name()];
        else
            return query_def_start();
    }
    
    // Fallback to traditional multi-room process if streamlined creation failed
    if (query_ghost() & GP_BODY)
        return "/d/Standard/login/bodies";
    else if (query_ghost() & GP_MANGLE)
        return "/d/Standard/login/mangle";
    else if (query_ghost() & GP_FEATURES)
        return "/d/Standard/login/features";
    else if (query_ghost() & GP_SKILLS)
        return "/d/Standard/login/skills";
    else if (query_ghost() == 0)
        return RACESTART[this_object()->query_race_name()];
    
    return "/d/Standard/login/bodies";
}
*/

// Alternative: Direct call from login process for new players

/*
// In the login sequence (e.g., in ghost_player.c ghost_start()):
void ghost_start()
{
    // ... existing code ...
    
    // For new players, use streamlined creation
    if (query_ghost() & GP_NEW)
    {
        if ("/d/Standard/login/create_character"->streamlined_ghost_creation(this_object()))
        {
            return; // Character created and moved to starting location
        }
        // If streamlined creation fails, fall back to traditional process
    }
    
    // ... rest of existing code ...
}
*/

// Example wizard command to test character creation:

/*
// Add to a wizard command file:
int cmd_test_chargen(string str)
{
    if (!query_wiz_level())
    {
        write("Permission denied.\n");
        return 1;
    }
    
    "/d/Standard/login/create_character"->test_character_creation();
    return 1;
}
*/

// Example standalone usage:

/*
void create_npc_with_defaults()
{
    object npc;
    
    npc = clone_object("/std/npc");
    npc->set_name("test_character");
    npc->set_ghost(GP_NEW);  // Mark as needing character creation
    
    "/d/Standard/login/create_character"->create_character(npc);
    
    // NPC now has race, gender, stats, height, weight, and adjectives assigned
}
*/
