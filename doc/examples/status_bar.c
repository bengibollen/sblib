/*
 * /doc/examples/status_bar.c
 *
 * Example demonstrating a colorful player status display using the color system
 */

#pragma strict_types
#include <color.h>

/**
 * Function name: generate_bar
 * Description  : Creates a colorful progress bar
 * Arguments    : int current - current value
 *                int max - maximum value
 *                int width - width of the bar in characters
 *                string color_full - color for the filled portion
 *                string color_empty - color for the unfilled portion
 * Returns      : string - formatted progress bar
 */
public string
generate_bar(int current, int max, int width, string color_full, string color_empty)
{
    if (max <= 0)
        max = 1; // Prevent division by zero
    
    if (current < 0)
        current = 0;
    else if (current > max)
        current = max;
    
    // Calculate how many chars should be filled
    int filled = (current * width) / max;
    
    // Generate the bar
    string bar = color_full;
    int i;
    for (i = 0; i < filled; i++) 
        bar += "█";
    
    bar += color_empty;
    
    for (; i < width; i++)
        bar += "█";
    
    return bar + C_RESET;
}

/**
 * Function name: player_status_bar
 * Description  : Creates a full status display for a player
 * Arguments    : string name - player name
 *                int hp - current hit points
 *                int max_hp - maximum hit points
 *                int mana - current mana points
 *                int max_mana - maximum mana points
 *                int stamina - current stamina points
 *                int max_stamina - maximum stamina points
 *                string status - optional status indicator
 * Returns      : string - formatted status bar
 */
public string
player_status_bar(string name, int hp, int max_hp, int mana, int max_mana, 
                 int stamina, int max_stamina, string status)
{
    // Create the health bar with color based on health percentage
    string hp_color;
    if (hp < max_hp / 4)
        hp_color = C_H_RED;
    else if (hp < max_hp / 2)
        hp_color = C_YELLOW;
    else
        hp_color = C_H_GREEN;
    
    string hp_bar = generate_bar(hp, max_hp, 10, hp_color, C_L_BLACK);
    
    // Create the mana bar
    string mana_bar = generate_bar(mana, max_mana, 10, C_H_BLUE, C_L_BLACK);
    
    // Create the stamina bar
    string stamina_bar = generate_bar(stamina, max_stamina, 10, C_H_YELLOW, C_L_BLACK);
    
    // Format HP/MP/SP values
    string hp_text = hp_color + hp + C_RESET + "/" + C_WHITE + max_hp + C_RESET;
    string mana_text = C_H_BLUE + mana + C_RESET + "/" + C_WHITE + max_mana + C_RESET;
    string stamina_text = C_H_YELLOW + stamina + C_RESET + "/" + C_WHITE + max_stamina + C_RESET;
    
    // Create status indicator if provided
    string status_text = "";
    if (stringp(status) && sizeof(status))
    {
        if (status == "poisoned")
            status_text = C_H_GREEN + " [POISONED]" + C_RESET;
        else if (status == "bleeding")
            status_text = C_H_RED + " [BLEEDING]" + C_RESET;
        else if (status == "confused")
            status_text = C_H_MAGENTA + " [CONFUSED]" + C_RESET;
        else if (status == "stunned")
            status_text = C_YELLOW + " [STUNNED]" + C_RESET;
        else
            status_text = C_WHITE + " [" + status + "]" + C_RESET;
    }
    
    // Compose the full status bar
    string player_name = C_PLAYERS + name + C_RESET + status_text;
    string stats_section = "HP: " + hp_text + " " + hp_bar + 
                          " MP: " + mana_text + " " + mana_bar +
                          " SP: " + stamina_text + " " + stamina_bar;
    
    // Create the border with distinct color
    string border = C_BLUE + "╔════════════════════════════════════════════════════════════════╗\n" +
                    "║ " + C_RESET + player_name + 
                    " " + C_BLUE + "╟────────────────────────────────────────────────────╢" + C_RESET + "\n" +
                    C_BLUE + "║ " + C_RESET + stats_section + 
                    " " + C_BLUE + "║\n" +
                    "╚════════════════════════════════════════════════════════════════╝" + C_RESET;
    
    return border;
}

/**
 * Function name: create
 * Description  : Tests the status bar system
 */
public void
create()
{
    write("\nPlayer Status Bar Examples:\n\n");
    
    // A healthy player
    write(player_status_bar("Aragorn", 120, 120, 80, 80, 100, 100, "") + "\n\n");
    
    // A wounded player
    write(player_status_bar("Frodo", 15, 60, 40, 50, 20, 70, "poisoned") + "\n\n");
    
    // A nearly defeated player
    write(player_status_bar("Boromir", 8, 100, 5, 60, 10, 90, "bleeding") + "\n\n");
}
