inherit "/std/creature";
inherit "/std/combat/unarmed";

#include <macros.h>
#include <const.h>
#include <wa_types.h>
#include <ss_types.h>

public void movedestroy();

/*
 * Define some attack and hitloc id's (only for our own benefit)
 */
#define A_HEADBUTT  0
#define A_KICK 1
#define A_BITE 2

#define H_HEAD 0
#define H_BODY 1


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
//    call_out((: movedestroy() :), 10); 

              // str, con, dex, int, wis, dis
    set_stats(({ 3,  3,  3,  1,  1,   2 }));

    set_hp(query_max_hp());

    set_skill(SS_DEFENCE, 30);
    set_skill(SS_SWIM, 80);

    set_attack_unarmed(A_HEADBUTT,  20, 3, W_BLUDGEON, 30, "headbutt");
    set_attack_unarmed(A_KICK, 40, 2, W_BLUDGEON,  40, "kick");
    set_attack_unarmed(A_BITE, 40, 2, W_BLUDGEON,  30, "teeth");

    set_hitloc_unarmed(H_HEAD, ({ 15, 25, 20, 20 }), 20, "head");
    set_hitloc_unarmed(H_BODY, ({ 10, 15, 30, 20 }), 80, "body");


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
