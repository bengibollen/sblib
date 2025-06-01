inherit "/std/room";

#include <stdproperties.h>
#include <macros.h>

string arr_line;

void create_room() {
    add_prop(OBJ_S_WIZINFO, "@@wizinfo");
    set_short("LPmud standard church");

    set_long("\n"+
	     "You are in the local village church.\n" +
	     "There is a deep pit in the center.\n" +
	     "There's a button on the wall labelled 'sheep' for some reason.\n");

//    add_exit("/d/Standard/start/mailroom","south","@@sblock");
    add_exit("/d/Standard/newbietown/church_plaza", "south");

    add_item(({"pit"}),"It is very deep and smells bad. You don't\n" +
		"dare go near the edge. The guy who made this room must\n" +
		"be nuts.\n");
    add_item(({"button"}),"It is very tempting to press it.\n");
    add_prop(ROOM_I_INSIDE, 1);
    add_prop(ROOM_I_LIGHT, 1);
}


query_no_snoop()
{
    return 1;
}


void init() {
    add_action("pray", "pray");
    add_action("rest", "rest");
    add_action("press_button", "press");
    ::init();
}


int pray()
{
    write("Since God doesn't exist, nothing happens.\n");
    if (!this_player()->query_invis())
	say( ({METNAME +" kneels down and mumbles.\n",
	       ART_NONMETNAME +" kneels down and mumbles.\n"}) );
    return 1;
}


int rest()
{
    write("Ah, wasn't that nice.\n");
    return 1;
}

int press_button(string str) {

    log_debug("Inventory: %O", all_inventory());

    if ("/w/debug/sheep" in map(all_inventory(), #'load_name))
    {
        write("You press the button and nothing happens.\n");
        say( QCTNAME(this_player()) + " presses the button and nothing happens.\n");
        return 1;
    }

    if (str == "button" || str == "sheep")
    {
        write("You press the button and a sheep appears.\n");
        say( QCTNAME(this_player()) + " presses the button and a sheep appears.\n");
        clone_object("/w/debug/sheep")->move(environment(this_player()));
        return 1;
    }

    write("Press what?\n");
    return 1;
}


string wizinfo() {
   return ("This is the starting location for all players in the standard\n"+
	   "distribution of this mudlib. It should be replaced as soon\n"+
	   "as possible by the local administrator.\n");
}


string sblock()
{
    return 0; 
}
