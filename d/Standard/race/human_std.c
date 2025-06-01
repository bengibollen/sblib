/*
  human_std.c

  This is the race object used for players of race: human
*/
inherit "/conf/race/generic";

static void start_player()
{
//    start_mail("/d/Standard/start/mailroom");
    log_debug("Starting player: %O", this_object());
    ::start_player();
}

public string query_race()
{
    return "human"; 
}

