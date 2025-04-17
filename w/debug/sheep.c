inherit "/std/creature";

#include <macros.h>
#include <const.h>
public void movedestroy();

void create_creature() {
    add_prop(LIVE_I_NEVERKNOWN, 1);
    set_race_name("sheep");
    add_adj("fluffy");
    add_adj("white");
    set_mm_out("disappears suddenly.\n");
    set_gender(G_FEMALE);
    set_short("fluffy white sheep");
    set_long("This is a fluffy sheep grazing peacefully.\n");
    set_chat_time(7);
    add_chat("Baaah!");
    add_chat("Bleeet!");
    add_chat("I am just a sheep.");
    call_out((: movedestroy() :), 10); 
}

public void movedestroy() {
    move_object(this_object(), "/w/debug/workroom");
    destruct(this_object());
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
