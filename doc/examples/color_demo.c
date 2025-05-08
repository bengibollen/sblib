/*
 * /doc/examples/color_demo.c
 *
 * Example demonstrating use of the Silver Bucket color system
 * Shows both predefined colors and dynamic color combinations
 */

#include <color.h>

create() {
    // Predefined colors can be used directly
    string predefined_demo = 
        C_RED + "This is standard red text. " + C_RESET +
        C_L_RED + "This is light red. " + C_RESET +
        C_BOLD + C_BLUE + "Bold blue using multiple codes. " + C_RESET;
    
    // Show example of predefined background colors
    string bg_demo = 
        C_BLACK + C_B_WHITE + "Black on white background. " + C_RESET +
        C_YELLOW + C_B_BLUE + "Yellow on blue background. " + C_RESET;
    
    // Examples of semantic colors
    string semantic_demo = 
        C_ERROR + "Error message! " + C_RESET +
        C_SUCCESS + "Success message! " + C_RESET +
        C_WARNING + "Warning message! " + C_RESET;
    
    // Examples using the dynamic color generation
    string dynamic_color1 = GENERATE_COLOR(ATTR_BOLD, FG_RED, BG_WHITE);
    string dynamic_color2 = GENERATE_COLOR(ATTR_UNDERLINE, FG_GREEN, BG_BLACK);
    string dynamic_color3 = GENERATE_COLOR(ATTR_ITALIC, FG_BLUE, BG_YELLOW);
    
    string dynamic_demo = 
        dynamic_color1 + "Bold red on white background. " + "\e[0m" +
        dynamic_color2 + "Underlined green on black background. " + "\e[0m" +
        dynamic_color3 + "Italic blue on yellow background. " + "\e[0m";
    
    // Define a new color combination and use it
    DEFINE_COLOR_COMBO("FANCY_TITLE", ATTR_BOLD, FG_CYAN, BG_BLUE);
    
    string combined_demo = 
        "%^FANCY_TITLE%^This is a fancy title with custom color combo%^RESET%^";
    
    // Demonstrate wrapping with colors
    string wrapped_text = 
        C_L_GREEN + "This is a long text with colors that will be wrapped " +
        "properly using the COLORIZE_WRAP function. The color codes don't " +
        "affect the wrapping calculation, so lines will be exactly the " +
        "specified width when displayed in the terminal." + C_RESET;
    
    // Write out all demos
    write("\nPredefined Colors Demo:\n" + predefined_demo + "\n\n");
    write("Background Colors Demo:\n" + bg_demo + "\n\n");
    write("Semantic Colors Demo:\n" + semantic_demo + "\n\n");
    write("Dynamic Color Generation Demo:\n" + dynamic_demo + "\n\n");
    write("Custom Color Combo Demo:\n" + COLORIZE(combined_demo) + "\n\n");
    write("Wrapped Text Demo (40 chars):\n" + COLORIZE_WRAP(wrapped_text, 40) + "\n\n");
}
