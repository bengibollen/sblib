#ifndef COLOR_H
#define COLOR_H

/*
 * /inc/color.h
 *
 * Header file for the Silver Bucket Mudlib color system.
 * Include this file to use the standard color functions and constants.
 */

// Path to the color utility object
#define COLOR_HANDLER "/inc/color"

// Function to colorize a string using the standard color mapping
#define COLORIZE(str) (call_other(COLOR_HANDLER, "colorize", (str)))

// Function to colorize and wrap a string at a specific width
#define COLORIZE_WRAP(str, width) (call_other(COLOR_HANDLER, "colorize", (str), (width)))

// Function to colorize and wrap a string with indentation
#define COLORIZE_WRAP_INDENT(str, width, indent) (call_other(COLOR_HANDLER, "colorize", (str), (width), (indent)))

// Function to strip all color codes from a string
#define STRIP_COLORS(str) (call_other(COLOR_HANDLER, "strip_colors", (str)))

// Function to wrap text without color processing
#define WRAP_TEXT(str, width) (call_other(COLOR_HANDLER, "wrap_text", (str), (width)))

// Generate custom color combinations
#define GENERATE_COLOR(attr, fg, bg) (call_other(COLOR_HANDLER, "generate_color", (attr), (fg), (bg)))

// Define a new color combination and add it to the color map
#define DEFINE_COLOR_COMBO(key, attr, fg, bg) (call_other(COLOR_HANDLER, "define_color_combo", (key), (attr), (fg), (bg)))

// Common color code macros for direct inclusion in strings
#define C_RESET     "%^RESET%^"
#define C_BOLD      "%^BOLD%^"
#define C_ITALIC    "%^ITALIC%^"
#define C_UNDERLINE "%^UNDERLINE%^"
#define C_BLINK     "%^BLINK%^"
#define C_INVERSE   "%^INVERSE%^"

// Standard colors
#define C_BLACK     "%^BLACK%^"
#define C_RED       "%^RED%^"
#define C_GREEN     "%^GREEN%^"
#define C_YELLOW    "%^YELLOW%^"
#define C_BLUE      "%^BLUE%^"
#define C_MAGENTA   "%^MAGENTA%^"
#define C_CYAN      "%^CYAN%^"
#define C_WHITE     "%^WHITE%^"

// Light/bright colors (using bold modifier)
#define C_L_BLACK   "%^L_BLACK%^"
#define C_L_RED     "%^L_RED%^"
#define C_L_GREEN   "%^L_GREEN%^"
#define C_L_YELLOW  "%^L_YELLOW%^"
#define C_L_BLUE    "%^L_BLUE%^"
#define C_L_MAGENTA "%^L_MAGENTA%^"
#define C_L_CYAN    "%^L_CYAN%^"
#define C_L_WHITE   "%^L_WHITE%^"

// High-intensity colors (using codes 90-97)
#define C_H_BLACK   "%^H_BLACK%^"
#define C_H_RED     "%^H_RED%^"
#define C_H_GREEN   "%^H_GREEN%^"
#define C_H_YELLOW  "%^H_YELLOW%^"
#define C_H_BLUE    "%^H_BLUE%^"
#define C_H_MAGENTA "%^H_MAGENTA%^"
#define C_H_CYAN    "%^H_CYAN%^"
#define C_H_WHITE   "%^H_WHITE%^"

// Background colors
#define C_B_BLACK   "%^B_BLACK%^"
#define C_B_RED     "%^B_RED%^"
#define C_B_GREEN   "%^B_GREEN%^"
#define C_B_YELLOW  "%^B_YELLOW%^"
#define C_B_BLUE    "%^B_BLUE%^"
#define C_B_MAGENTA "%^B_MAGENTA%^"
#define C_B_CYAN    "%^B_CYAN%^"
#define C_B_WHITE   "%^B_WHITE%^"

// High-intensity background colors (using codes 100-107)
#define C_B_H_BLACK   "%^B_H_BLACK%^"
#define C_B_H_RED     "%^B_H_RED%^"
#define C_B_H_GREEN   "%^B_H_GREEN%^"
#define C_B_H_YELLOW  "%^B_H_YELLOW%^"
#define C_B_H_BLUE    "%^B_H_BLUE%^"
#define C_B_H_MAGENTA "%^B_H_MAGENTA%^"
#define C_B_H_CYAN    "%^B_H_CYAN%^"
#define C_B_H_WHITE   "%^B_H_WHITE%^"

// Semantic colors
#define C_ERROR     "%^ERROR%^"
#define C_WARNING   "%^WARNING%^"
#define C_SUCCESS   "%^SUCCESS%^"
#define C_INFO      "%^INFO%^"
#define C_NOTICE    "%^NOTICE%^"

// Game-specific semantic colors
#define C_EXITS     "%^EXITS%^"
#define C_ITEMS     "%^ITEMS%^"
#define C_NPCS      "%^NPCS%^"
#define C_PLAYERS   "%^PLAYERS%^"
#define C_SPELL     "%^SPELL%^"
#define C_DAMAGE    "%^DAMAGE%^"
#define C_HEAL      "%^HEAL%^"

/* ANSI Color Code Constants for use with GENERATE_COLOR */
// Attributes
#define ATTR_NORMAL     0
#define ATTR_BOLD       1
#define ATTR_ITALIC     3
#define ATTR_UNDERLINE  4
#define ATTR_BLINK      5
#define ATTR_INVERSE    7

// Foreground colors
#define FG_BLACK        30
#define FG_RED          31
#define FG_GREEN        32
#define FG_YELLOW       33
#define FG_BLUE         34
#define FG_MAGENTA      35
#define FG_CYAN         36
#define FG_WHITE        37
#define FG_DEFAULT      0  // No foreground color specified

// High intensity foreground colors
#define FG_H_BLACK      90
#define FG_H_RED        91
#define FG_H_GREEN      92
#define FG_H_YELLOW     93
#define FG_H_BLUE       94
#define FG_H_MAGENTA    95
#define FG_H_CYAN       96
#define FG_H_WHITE      97

// Background colors
#define BG_BLACK        40
#define BG_RED          41
#define BG_GREEN        42
#define BG_YELLOW       43
#define BG_BLUE         44
#define BG_MAGENTA      45
#define BG_CYAN         46
#define BG_WHITE        47
#define BG_DEFAULT      0  // No background color specified

// High intensity background colors
#define BG_H_BLACK      100
#define BG_H_RED        101
#define BG_H_GREEN      102
#define BG_H_YELLOW     103
#define BG_H_BLUE       104
#define BG_H_MAGENTA    105
#define BG_H_CYAN       106
#define BG_H_WHITE      107

#endif /* COLOR_H */
