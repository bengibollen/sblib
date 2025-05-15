inherit "/std/weapon";

#include <wa_types.h>      /* contains weapon-related definitions */
#include <formulas.h>        /* contains formulas for weight values, etc */
#include <stdproperties.h> /* OBJ_I_VALUE and those properties    */
#include <macros.h>

/* Define the to-hit and penetration values. */
#define HIT 8
#define PEN 9

void create_weapon()
{
    set_name("sword");
    set_pname("swords");
    set_short("training sword");
    set_pshort("training swords");
    set_adj("training");
    set_long("This is a training sword, it is not very sharp and is made of wood.\n");

    /* Now we want to set the 'to hit' value and 'penetration value' */
    set_hit(HIT);
    set_pen(PEN);

    /* The weapon type and the type of damage done by this weapon */
    set_wt(W_KNIFE);            /* It's of 'knife' type */
    set_dt(W_SLASH | W_IMPALE); /* You can both 'slash' and 'impale' with it */

    /* Last, how shall it be wielded? */
    set_hands(W_ANYH); /* You can wield it in any hand. */

    /* Now give the weapon some value, weight and volume */
    /* use standard functions, randomize to hide actual WC values */
    add_prop(OBJ_I_VALUE, F_VALUE_WEAPON(HIT, PEN) + random(20) - 10);
    add_prop(OBJ_I_WEIGHT, F_WEIGHT_DEFAULT_WEAPON(PEN, W_KNIFE) + random(50) - 25);

    /* calculate volume from weight */
    add_prop(OBJ_I_VOLUME, query_prop(OBJ_I_WEIGHT) / 5);
}
