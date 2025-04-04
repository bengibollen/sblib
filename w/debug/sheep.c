inherit "/std/creature";

void create_creature() {
    set_name("sheep");
    set_short("a fluffy sheep\n");
    set_long("This is a fluffy sheep grazing peacefully.\n");
    set_chat_time(7);
    add_chat("Baaah!");
    add_chat("Bleeet!");
    add_chat("I am just a sheep.");
}

public int communicate(string str) /* Must have it here for special with ' */
{
    string verb;

    verb = query_verb();
    if (str == 0)
        str = "";
    if (strlen(verb) && verb[0] == "'"[0])
        str = extract(verb, 1) + " " + str;

    say(QCTNAME(this_object()) + " @@race_sound:" + object_name(this_object()) +
	"@@: " + str + "\n");
    write("Ok.\n");
    return 1;
}