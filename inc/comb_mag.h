/* /sys/comb_mag.h
 *
 * This file conmtains some neat definitions you might find useful in combat
 * or magic operations.
 *
 */

#ifndef COMB_MAG_DEF
#define COMB_MAG_DEF

#ifndef PROP_DEF
#include "/sys/stdproperties.h"
#endif

/* Object No ATTACK, and Room No ATTACK */
#define ONATTACK(ob)	({int}) ob->query_prop(OBJ_I_NO_ATTACK)
#define RNATTACK(ob)	({int}) ob->query_prop(ROOM_I_NO_ATTACK)

/* Object No Magic ATTACK, and Room No Magic ATTACK */
#define ONMATTACK(ob)	({int}) ob->query_prop(OBJ_I_NO_MAGIC_ATTACK)
#define RNMATTACK(ob)	({int}) ob->query_prop(ROOM_I_NO_MAGIC_ATTACK)

/* Object No MAGIC, and Room No MAGIC */
#define ONMAGIC(ob)	({int}) ob->query_prop(OBJ_I_NO_MAGIC)
#define RNMAGIC(ob)	({int}) ob->query_prop(ROOM_I_NO_MAGIC)

/* Object No TELEPORT, and Room No TELEPORT */
#define ONTELEPORT(ob)	({int}) ob->query_prop(OBJ_I_NO_TELEPORT)
#define RNTELEPORT(ob)	({int}) ob->query_prop(ROOM_I_NO_TELEPORT)

/* Not Possible ATTACK, is it possible to attack this object in this room? */
#define NPATTACK(ob)	(ONATTACK(ob) || (environment(ob) && \
			    RNATTACK(environment(ob))))

/* Not Possible MAGIC, is it possible to magic this object in this room? */
#define NPMAGIC(ob)	(ONMAGIC(ob) || (environment(ob) && \
			    RNMAGIC(environment(ob))))

/* Not Possible Magic ATTACK, is it possible to attack with magic, this object
 * in this room? */
#define NPMATTACK(ob)   (NPATTACK(ob) || NPMAGIC(ob) || ONMATTACK(ob) || \
			    (environment(ob) && RNMATTACK(environment(ob))))

/* Not Possible TELEPORT, is it possible to teleport this object in this room? */
#define NPTELEPORT(ob)  (ONTELEPORT(ob) || (environment(this_player()) && \
			    RNTELEPORT(environment(this_player()))))

/* No definitions beyond this line. */
#endif

