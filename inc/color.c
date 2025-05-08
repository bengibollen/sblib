/*
 * /inc/color.c
 *
 * Central color management system for Silver Bucket Mudlib
 * 
 * This module provides standardized color codes and utility functions
 * for handling terminal colors throughout the codebase.
 */

#pragma strict_types

#include <macros.h>

// Global color mapping
static mapping COLOR_MAP = ([
    // Reset and formatting codes
    "RESET"     : "\e[0m",
    "BOLD"      : "\e[1m",
    "ITALIC"    : "\e[3m",
    "UNDERLINE" : "\e[4m",
    "BLINK"     : "\e[5m",
    "INVERSE"   : "\e[7m",
    
    // Standard colors
    "BLACK"     : "\e[30m",
    "RED"       : "\e[31m",
    "GREEN"     : "\e[32m",
    "YELLOW"    : "\e[33m",
    "BLUE"      : "\e[34m",
    "MAGENTA"   : "\e[35m",
    "CYAN"      : "\e[36m",
    "WHITE"     : "\e[37m",
    
    // Light/bright colors - traditional format (using bold modifier)
    "L_BLACK"   : "\e[1;30m",  // Usually appears as dark grey
    "L_RED"     : "\e[1;31m",
    "L_GREEN"   : "\e[1;32m",
    "L_YELLOW"  : "\e[1;33m",
    "L_BLUE"    : "\e[1;34m",
    "L_MAGENTA" : "\e[1;35m",
    "L_CYAN"    : "\e[1;36m",
    "L_WHITE"   : "\e[1;37m",
    
    // Light/bright colors - alternative format (using codes 90-97)
    "H_BLACK"   : "\e[90m",   // High intensity black (grey)
    "H_RED"     : "\e[91m",   // High intensity red
    "H_GREEN"   : "\e[92m",   // High intensity green
    "H_YELLOW"  : "\e[93m",   // High intensity yellow
    "H_BLUE"    : "\e[94m",   // High intensity blue
    "H_MAGENTA" : "\e[95m",   // High intensity magenta
    "H_CYAN"    : "\e[96m",   // High intensity cyan
    "H_WHITE"   : "\e[97m",   // High intensity white
    
    // Background colors
    "B_BLACK"   : "\e[40m",
    "B_RED"     : "\e[41m",
    "B_GREEN"   : "\e[42m",
    "B_YELLOW"  : "\e[43m",
    "B_BLUE"    : "\e[44m",
    "B_MAGENTA" : "\e[45m", 
    "B_CYAN"    : "\e[46m",
    "B_WHITE"   : "\e[47m",
    
    // Light background colors (using codes 100-107)
    "B_H_BLACK"   : "\e[100m",  // High intensity background black
    "B_H_RED"     : "\e[101m",  // High intensity background red
    "B_H_GREEN"   : "\e[102m",  // High intensity background green
    "B_H_YELLOW"  : "\e[103m",  // High intensity background yellow
    "B_H_BLUE"    : "\e[104m",  // High intensity background blue
    "B_H_MAGENTA" : "\e[105m",  // High intensity background magenta
    "B_H_CYAN"    : "\e[106m",  // High intensity background cyan
    "B_H_WHITE"   : "\e[107m",  // High intensity background white
    
    // Common semantic color combinations
    "ERROR"     : "\e[91m",     // High intensity red for errors
    "WARNING"   : "\e[93m",     // High intensity yellow for warnings
    "SUCCESS"   : "\e[92m",     // High intensity green for success
    "INFO"      : "\e[96m",     // High intensity cyan for information
    "NOTICE"    : "\e[95m",     // High intensity magenta for notices
    
    // Game-specific semantic colors
    "EXITS"     : "\e[32m",     // Green for exits
    "ITEMS"     : "\e[33m",     // Yellow for items
    "NPCS"      : "\e[96m",     // High intensity cyan for NPCs
    "PLAYERS"   : "\e[97m",     // High intensity white for players
    "SPELL"     : "\e[95m",     // High intensity magenta for magic
    "DAMAGE"    : "\e[91m",     // High intensity red for damage
    "HEAL"      : "\e[92m"      // High intensity green for healing
]);

/*
 * Function name: query_color_map
 * Description  : Returns the color mapping. This allows other objects
 *                to access and use the standard color codes.
 * Returns      : mapping - The color mapping
 */
public mapping
query_color_map()
{
    return COLOR_MAP + ([]);  // Return a copy to prevent direct modification
}

/*
 * Function name: colorize
 * Description  : Applies color codes to a string and performs proper text wrapping
 * Arguments    : string text - The text to colorize
 *                int wrap_width - Optional width to wrap the text at
 *                int indent - Optional indentation for wrapped lines
 * Returns      : string - The colorized and properly wrapped text
 */
public varargs string 
colorize(string text, int wrap_width = 0, int indent = 0)
{
    if (!stringp(text))
        return "";
        
    return terminal_colour(text, COLOR_MAP, wrap_width, indent);
}

/*
 * Function name: strip_colors
 * Description  : Removes all color codes from a string
 * Arguments    : string text - The text to process
 * Returns      : string - The text without color codes
 */
public string
strip_colors(string text)
{
    if (!stringp(text))
        return "";
        
    return terminal_colour(text, ([]), 0, 0);
}

/*
 * Function name: add_color
 * Description  : Add a custom color code to the color mapping
 * Arguments    : string key - The color code key (without %^ markers)
 *                string value - The ANSI sequence to use
 * Returns      : int - 1 for success, 0 for failure
 */
public int
add_color(string key, string value)
{
    if (!stringp(key) || !stringp(value))
        return 0;
        
    COLOR_MAP[key] = value;
    return 1;
}

/*
 * Function name: wrap_text
 * Description  : A simple wrapper around terminal_colour for just wrapping text
 *                without any color processing.
 * Arguments    : string text - The text to wrap
 *                int width - The width to wrap at
 *                int indent - Optional indentation for wrapped lines
 * Returns      : string - The wrapped text
 */
public varargs string
wrap_text(string text, int width, int indent = 0)
{
    if (!stringp(text) || width <= 0)
        return text;
        
    return terminal_colour(text, 0, width, indent);
}

/*
 * Function name: generate_color
 * Description  : Dynamically generates an ANSI color code based on attribute, 
 *                foreground and background settings.
 * Arguments    : int attr - Style attribute (0=normal, 1=bold, 3=italic, 4=underline, etc.)
 *                int fg - Foreground color (30-37)
 *                int bg - Background color (40-47)
 * Returns      : string - The ANSI escape sequence for the specified color combination
 */
public string
generate_color(int attr, int fg, int bg)
{
    string code = "\e[";
    
    // Add attribute if specified (non-zero)
    if (attr > 0) {
        code += attr;
        // Add separator if we'll be adding more codes
        if (fg > 0 || bg > 0) {
            code += ";";
        }
    }
    
    // Add foreground if specified
    if (fg >= 30 && fg <= 37) {
        code += fg;
        // Add separator if we'll be adding background
        if (bg > 0) {
            code += ";";
        }
    }
    
    // Add background if specified
    if (bg >= 40 && bg <= 47) {
        code += bg; 
    }
    
    // Complete the sequence
    code += "m";
    
    return code;
}

/*
 * Function name: define_color_combo
 * Description  : Adds a new color combination to the color map
 * Arguments    : string key - The key name for the new color
 *                int attr - Style attribute (0=normal, 1=bold, 3=italic, 4=underline, etc.)
 *                int fg - Foreground color (30-37)
 *                int bg - Background color (40-47)
 * Returns      : int - 1 for success, 0 for failure
 */
public int
define_color_combo(string key, int attr, int fg, int bg)
{
    if (!stringp(key))
        return 0;
        
    COLOR_MAP[key] = generate_color(attr, fg, bg);
    return 1;
}
