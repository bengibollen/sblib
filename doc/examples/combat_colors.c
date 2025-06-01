/*
 * /doc/examples/combat_colors.c
 *
 * Example demonstrating the use of the color system for combat messages
 */

#pragma strict_types
#include <color.h>

/**
 * Function name: format_damage_message
 * Description  : Creates a colorized combat damage message
 * Arguments    : string attacker - name of the attacking entity
 *                string defender - name of the defending entity
 *                string weapon - weapon or attack method used
 *                int damage - amount of damage dealt
 *                int critical - whether this was a critical hit (0/1)
 * Returns      : string - The formatted combat message
 */
public string
format_damage_message(string attacker, string defender, string weapon, int damage, int critical)
{
    string msg;
    
    // Add attacker name in player or NPC color depending on capitalization
    // (Simple heuristic: player names are capitalized)
    if (attacker[0] >= 'A' && attacker[0] <= 'Z')
        msg = C_PLAYERS + attacker + C_RESET;
    else
        msg = C_NPCS + attacker + C_RESET;
    
    // Add attack action
    msg += " " + (critical ? "critically strikes" : "hits");
    
    // Add defender name
    if (defender[0] >= 'A' && defender[0] <= 'Z')
        msg += " " + C_PLAYERS + defender + C_RESET;
    else
        msg += " " + C_NPCS + defender + C_RESET;
    
    // Add weapon information
    msg += " with " + C_ITEMS + weapon + C_RESET;
    
    // Add damage amount with color based on severity
    string damage_text;
    if (damage < 5)
        damage_text = C_L_RED + "(" + damage + " damage)" + C_RESET;
    else if (damage < 15)
        damage_text = C_RED + "(" + damage + " damage)" + C_RESET;
    else if (damage < 30)
        damage_text = C_H_RED + "(" + damage + " damage)" + C_RESET;
    else
        damage_text = C_H_RED + C_BOLD + "(" + damage + " damage)" + C_RESET;
    
    // Add critical hit indicator for critical hits
    if (critical)
        damage_text = C_YELLOW + "**" + C_RESET + damage_text + C_YELLOW + "**" + C_RESET;
    
    return COLORIZE_WRAP(msg + " " + damage_text + ".", 78);
}

/**
 * Function name: format_healing_message
 * Description  : Creates a colorized healing message
 * Arguments    : string healer - name of the healing entity
 *                string target - name of the receiving entity
 *                string method - healing method used
 *                int amount - amount of healing done
 * Returns      : string - The formatted healing message
 */
public string
format_healing_message(string healer, string target, string method, int amount)
{
    string msg;
    
    // Add healer name
    if (healer[0] >= 'A' && healer[0] <= 'Z')
        msg = C_PLAYERS + healer + C_RESET;
    else
        msg = C_NPCS + healer + C_RESET;
    
    // Add healing action
    msg += " heals ";
    
    // Add target name
    if (target[0] >= 'A' && target[0] <= 'Z')
        msg += C_PLAYERS + target + C_RESET;
    else
        msg += C_NPCS + target + C_RESET;
    
    // Add method information
    msg += " using " + C_SPELL + method + C_RESET;
    
    // Add healing amount with color based on magnitude
    string heal_text;
    if (amount < 5)
        heal_text = C_L_GREEN + "(" + amount + " health)" + C_RESET;
    else if (amount < 15)
        heal_text = C_GREEN + "(" + amount + " health)" + C_RESET;
    else if (amount < 30)
        heal_text = C_H_GREEN + "(" + amount + " health)" + C_RESET;
    else
        heal_text = C_H_GREEN + C_BOLD + "(" + amount + " health)" + C_RESET;
    
    return COLORIZE_WRAP(msg + " " + heal_text + ".", 78);
}

/**
 * Function name: create
 * Description  : Tests the combat message formatting functions
 */
public void
create()
{
    // Test some combat messages
    write("\nCombat Messages Example:\n\n");
    
    write(format_damage_message("Danne", "orc warrior", "longsword", 8, 0) + "\n");
    write(format_damage_message("fire drake", "Bofur", "fiery breath", 24, 0) + "\n");
    write(format_damage_message("Gimli", "cave troll", "battle axe", 32, 1) + "\n");
    
    // Test some healing messages
    write("\nHealing Messages Example:\n\n");
    
    write(format_healing_message("Gandalf", "Frodo", "healing spell", 12) + "\n");
    write(format_healing_message("cleric", "injured knight", "holy light", 35) + "\n");
}
