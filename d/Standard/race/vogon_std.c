/*
  vogon_std.c

  This is the race object used for players of race: vogon
*/
inherit "/conf/race/generic";

start_player()
{
    start_mail("/d/Standard/start/mailroom");
    ::start_player();
}

query_race() { return "vogon"; }


