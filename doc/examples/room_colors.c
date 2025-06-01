/*
 * /doc/examples/room_colors.c
 *
 * Example demonstrating enhanced room descriptions using the color system
 */

#pragma strict_types
#include <color.h>

/**
 * Function name: format_room_title
 * Description  : Creates a nicely formatted, colorized room title
 * Arguments    : string title - the room title
 *                string area - the area name
 *                string terrain - terrain type for color selection
 * Returns      : string - formatted room title
 */
public string
format_room_title(string title, string area, string terrain)
{
    string color;
    
    // Select color based on terrain type
    switch(terrain)
    {
        case "forest":    color = C_H_GREEN; break;
        case "mountain":  color = C_L_WHITE; break;
        case "desert":    color = C_YELLOW; break;
        case "swamp":     color = C_GREEN; break;
        case "cave":      color = C_L_BLACK; break;
        case "dungeon":   color = C_L_RED; break;
        case "city":      color = C_CYAN; break;
        case "castle":    color = C_L_BLUE; break;
        case "indoor":    color = C_H_YELLOW; break;
        default:          color = C_WHITE;
    }
    
    // Format the title with color
    string formatted = color + title + C_RESET;
    
    // Add area name if provided
    if (stringp(area) && sizeof(area))
        formatted += " " + C_B_BLACK + C_H_WHITE + " " + area + " " + C_RESET;
        
    return formatted;
}

/**
 * Function name: format_room_description
 * Description  : Creates a colorized room description with highlighted keywords
 * Arguments    : string desc - the base room description
 *                mapping highlights - mapping of words to highlight with their colors
 * Returns      : string - colorized room description
 */
public string
format_room_description(string desc, mapping highlights)
{
    string result = desc;
    
    // Apply each highlight from the mapping
    if (mappingp(highlights))
    {
        foreach (string word, string color in highlights)
        {
            // If we find the word in the description, highlight it
            // We need to be careful not to replace parts of other words
            int pos = 0;
            while ((pos = strstr(result, word, pos)) != -1)
            {
                // Check if this is a standalone word (preceded and followed by non-alphanumeric)
                if ((pos == 0 || !IS_ALPHA_NUM(result[pos-1])) && 
                    (pos + sizeof(word) >= sizeof(result) || !IS_ALPHA_NUM(result[pos + sizeof(word)])))
                {
                    // Replace the word with its colorized version
                    result = result[0..pos-1] + color + word + C_RESET + 
                             result[pos + sizeof(word)..];
                    
                    // Skip past the inserted color codes
                    pos += sizeof(word) + sizeof(color) + sizeof(C_RESET);
                }
                else
                {
                    // Move past this occurrence and continue searching
                    pos += 1;
                }
            }
        }
    }
    
    return COLORIZE_WRAP(result, 78, 2);
}

/**
 * Function name: format_exits
 * Description  : Creates colorized exit descriptions
 * Arguments    : string *exits - array of available exits
 *                mapping hidden_exits - mapping of hidden exits and their difficulty
 * Returns      : string - formatted exits description
 */
public string
format_exits(string *exits, mapping hidden_exits)
{
    string result = "";
    
    if (!arrayp(exits) || !sizeof(exits))
        return "There are no obvious exits.\n";
    
    // Format obvious exits
    if (sizeof(exits) == 1)
        result = "There is one obvious exit: ";
    else
        result = "There are " + sizeof(exits) + " obvious exits: ";
    
    // Add colorized exits
    for (int i = 0; i < sizeof(exits); i++)
    {
        result += C_EXITS + exits[i] + C_RESET;
        if (i < sizeof(exits) - 1)
            result += (i == sizeof(exits) - 2) ? " and " : ", ";
    }
    
    // Add wizard-visible hidden exits if any
    if (mappingp(hidden_exits) && sizeof(hidden_exits))
    {
        result += "\n" + C_L_BLACK + "There are also some hidden exits: ";
        int i = 0;
        foreach (string exit, int difficulty in hidden_exits)
        {
            // Color code based on difficulty
            string diff_color;
            if (difficulty > 80)
                diff_color = C_H_RED;
            else if (difficulty > 50)
                diff_color = C_YELLOW;
            else
                diff_color = C_GREEN;
                
            result += diff_color + exit + C_RESET + C_L_BLACK;
            
            if (i < sizeof(hidden_exits) - 1)
                result += (i == sizeof(hidden_exits) - 2) ? " and " : ", ";
            i++;
        }
        result += C_RESET;
    }
    
    return result + ".\n";
}

/**
 * Function name: create
 * Description  : Tests the room formatting functions
 */
public void
create()
{
    write("\nRoom Description Example:\n\n");
    
    // Room title
    write(format_room_title("Deep Forest Glade", "Elven Woods", "forest") + "\n");
    
    // Room description with highlighted keywords
    string desc = "A peaceful glade nestled deep within the ancient forest. "
                 "Tall oak trees form a protective circle around a small, clear pond. "
                 "Sunlight filters through the dense canopy, creating dappled patterns "
                 "on the forest floor. Several deer quietly drink from the pond, while "
                 "colorful butterflies flutter about. A small stone altar stands at the "
                 "northern edge of the clearing, covered in strange elven runes.";
    
    mapping highlights = ([
        "glade" : C_H_GREEN,
        "oak trees" : C_GREEN,
        "pond" : C_BLUE,
        "sunlight" : C_YELLOW,
        "deer" : C_L_YELLOW,
        "butterflies" : C_H_MAGENTA,
        "altar" : C_L_CYAN,
        "elven runes" : C_H_CYAN + C_UNDERLINE
    ]);
    
    write(format_room_description(desc, highlights) + "\n");
    
    // Exits
    string *exits = ({ "north", "east", "southwest" });
    mapping hidden_exits = ([ "down" : 75, "portal" : 90 ]);
    
    write(format_exits(exits, hidden_exits));
}
