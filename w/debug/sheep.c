inherit "/std/creature";

#include <macros.h>
#include <const.h>

void create_creature() {
    set_name("sheep");
    add_adj("fluffy");
    add_adj("white");
    set_gender(G_FEMALE);
    set_short("a fluffy sheep\n");
    set_long("This is a fluffy sheep grazing peacefully.\n");
    set_chat_time(7);
    add_chat("Baaah!");
    add_chat("Bleeet!");
    add_chat("I am just a sheep.");
}

public varargs int communicate(string str) /* Must have it here for special with ' */
{
    string verb;

    verb = query_verb();
    if (str == 0)
        str = "";
    if (sizeof(verb) && verb[0] == "'"[0])
        str = verb[1..] + " " + str;

    say(QCTNAME(this_object()) + " @@race_sound:" + object_name(this_object()) +
	"@@: " + str + "\n");
    write("Ok.\n");
    return 1;
}
