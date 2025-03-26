/*
 * /inc/libfiles.h
 *
 * This inclusion files contains the filenames of most mudlib modules in
 * definitions. If, except for #include or inherit, you ever need to make
 * a reference to a mudlib file, the name should be in here.
 */

#ifndef FILES_DEFINED
#define FILES_DEFINED

/* The section /cmd */

#define CMD_LIVE_INFO      ("/std/cmd/live/info")
#define CMD_LIVE_ITEMS     ("/std/cmd/live/items")
#define CMD_LIVE_MAGIC     ("/std/cmd/live/magic")
#define CMD_LIVE_SOCIAL    ("/std/cmd/live/social")
#define CMD_LIVE_SPEECH    ("/std/cmd/live/speech")
#define CMD_LIVE_STATE     ("/std/cmd/live/state")
#define CMD_LIVE_THIEF     ("/std/cmd/live/thief")
#define CMD_LIVE_THINGS    ("/std/cmd/live/things")

#define COMMAND_DRIVER     ("/std/command_driver")
#define SOUL_CMD           ("/std/cmd/std/soul_cmd")
#define TRACER_TOOL_SOUL   ("/std/cmd/std/tracer_tool")

#define WIZ_CMD_MORTAL     ("/std/cmd/wiz/mortal")
#define WIZ_CMD_APPRENTICE ("/std/cmd/wiz/apprentice")
#define WIZ_CMD_PILGRIM    ("/std/cmd/wiz/pilgrim")
#define WIZ_CMD_RETIRED    ("/std/cmd/wiz/retired")
#define WIZ_CMD_NORMAL     ("/std/cmd/wiz/normal")
#define WIZ_CMD_HELPER     ("/std/cmd/wiz/helper")
#define WIZ_CMD_MAGE       ("/std/cmd/wiz/mage")
#define WIZ_CMD_LORD       ("/std/cmd/wiz/lord")
#define WIZ_CMD_ARCH       ("/std/cmd/wiz/arch")
#define WIZ_CMD_KEEPER     ("/std/cmd/wiz/keeper")

#define JUNIOR_TOOL        ("/std/cmd/wiz/junior_tool")
#define JUNIOR_SHADOW      ("/std/cmd/wiz/junior_shadow")
#define MBS_SOUL           ("/std/cmd/wiz/mbs")

/* The section /lib */

#define BANK_LIBRARY       ("/lib/bank")
#define CACHE_LIBRARY      ("/lib/cache")
#define COMMANDS_LIBRARY   ("/lib/commands")
#define GUILD_LIBRARY      ("/lib/guild_support")
#define HERB_LIBRARY       ("/lib/herb_library")
#define HOLDABLE_LIBRARY   ("/lib/holdable_item")
#define NO_DECAY_LIBRARY   ("/lib/no_skill_decay")
#define PUB_LIBRARY        ("/lib/pub")
#define SHOP_LIBRARY       ("/lib/shop")
#define SKILL_LIBRARY      ("/lib/skill_raise")
#define STORE_LIBRARY      ("/lib/store_support")
#define TRADE_LIBRARY      ("/lib/trades")
#define WEARABLE_LIBRARY   ("/lib/wearable_item")

/* The section /obj */

#define EDITOR_OBJECT      ("/obj/edit")
#define NAMETAG_OBJECT     ("/obj/know_me")
#define POSSESSION_OBJECT  ("/obj/possob")
#define REMOTE_NPC_OBJECT  ("/obj/remote_npc")

/* The section /secure */

#define APPLICATION_PLAYER ("/secure/application_player")
#define BOARD_CENTRAL      ("/secure/mbs_central")
#define DOCMAKER           ("/secure/docmake")
#define EDITOR_SECURITY    ("/secure/editor")
#define FINGER_PLAYER      ("/secure/finger_player")
#define GAMEINFO_OBJECT    ("/secure/gameinfo_player")
#define GARBAGE_COLLECTOR  ("/secure/master/mail_gc")
#define LOGIN_OBJECT       ("/secure/login")
#define MAIL_CHECKER       ("/secure/mail_checker")
#define MAIL_READER        ("/secure/mail_reader")
#define PLAYER_TOOL        ("/secure/player_tool")
#define PURGE_OBJECT       ("/secure/master/purge")
#define QUEUE              ("/secure/queue")
#define SECURITY           ("/secure/master")
#define SIMUL_EFUN         ("/secure/simul_efun")
#define SRCMAN             ("/secure/srcman")
#define VBFC_OBJECT        ("/secure/vbfc_object")

/* The section /std */

#define ARMOUR_OBJECT      ("/std/armour")
#define ARROW_OBJECT       ("/std/arrow")
#define BOARD_OBJECT       ("/std/board")
#define BOOK_OBJECT        ("/std/book")
#define BOW_OBJECT         ("/std/bow")
#define CHUMLOCK_OBJECT    ("/std/combat/chumlock")
#define COINS_OBJECT       ("/std/coins")
#define CONTAINER_OBJECT   ("/std/container")
#define CORPSE_OBJECT      ("/std/corpse")
#define CREATURE_OBJECT    ("/std/creature")
#define DOMAIN_LINK_OBJECT ("/std/domain_link")
#define DOOR_OBJECT        ("/std/door")
#define DRINK_OBJECT       ("/std/drink")
#define FOOD_OBJECT        ("/std/food")
#define HEAP_OBJECT        ("/std/heap")
#define HERB_OBJECT        ("/std/herb")
#define KEY_OBJECT         ("/std/key")
#define LAUNCH_OBJECT      ("/std/launch_weapon")
#define LEFTOVER_OBJECT    ("/std/leftover")
#define LIVING_OBJECT      ("/std/living")
#define MOBILE_OBJECT      ("/std/mobile")
#define MONSTER_OBJECT     ("/std/monster")
#define OBJECT_OBJECT      ("/std/object")
#define PARALYZE_OBJECT    ("/std/paralyze")
#define PLAYER_OBJECT      ("/std/player")
#define PLAYER_SEC_OBJECT  ("/std/player_sec")
#define PLAYER_PUB_OBJECT  ("/std/player")
#define POISON_OBJECT      ("/std/poison_effect")
#define POTION_OBJECT      ("/std/potion")
#define PROJECTILE_OBJECT  ("/std/projectile")
#define RECEPTACLE_OBJECT  ("/std/receptacle")
#define RESISTANCE_OBJECT  ("/std/resistance")
#define ROOM_OBJECT        ("/std/room")
#define ROPE_OBJECT        ("/std/rope")
#define SCROLL_OBJECT      ("/std/scroll")
#define SHADOW_OBJECT      ("/std/shadow")
#define SPELLS_OBJECT      ("/std/spells")
#define TORCH_OBJECT       ("/std/torch")
#define WEAPON_OBJECT      ("/std/weapon")
#define WORKROOM_OBJECT    ("/std/workroom")

/* The section /sys */
#define MANCTRL            ("/sys/global/manpath")


/* Determine the type of an object based on its constructors. */
#define IS_CREATE_SOME(ob, func, file) (function_exists((func), (ob)) == (file))
#define IS_CREATE_CONTAINER(ob, file) IS_CREATE_SOME((ob), "create_container", (file))
#define IS_CREATE_OBJECT(ob, file)    IS_CREATE_SOME((ob), "create_object", (file))
#define IS_CREATE_HEAP(ob, file)      IS_CREATE_SOME((ob), "create_heap", (file))

#define IS_ARMOUR_OBJECT(ob)     IS_CREATE_OBJECT((ob), ARMOUR_OBJECT)
#define IS_ARROW_OBJECT(ob)      IS_CREATE_SOME((ob), "create_projectile", ARROW_OBJECT)
#define IS_BOARD_OBJECT(ob)      IS_CREATE_OBJECT((ob), BOARD_OBJECT)
#define IS_BOOK_OBJECT(ob)       IS_CREATE_OBJECT((ob), BOOK_OBJECT)
#define IS_BOW_OBJECT(ob)        IS_CREATE_SOME((ob), "create_launch_weapon", BOW_OBJECT)
#define IS_COINS_OBJECT(ob)      IS_CREATE_HEAP((ob), COINS_OBJECT)
#define IS_CONTAINER_OBJECT(ob)  IS_CREATE_OBJECT((ob), CONTAINER_OBJECT)
#define IS_CORPSE_OBJECT(ob)     IS_CREATE_CONTAINER((ob), CORPSE_OBJECT)
#define IS_CREATURE_OBJECT(ob)   IS_CREATE_SOME((ob), "create_mobile", CREATURE_OBJECT)
#define IS_DOOR_OBJECT(ob)       IS_CREATE_OBJECT((ob), DOOR_OBJECT)
#define IS_DRINK_OBJECT(ob)      IS_CREATE_HEAP((ob), DRINK_OBJECT)
#define IS_FOOD_OBJECT(ob)       IS_CREATE_HEAP((ob), FOOD_OBJECT)
#define IS_HEAP_OBJECT(ob)       IS_CREATE_OBJECT((ob), HEAP_OBJECT)
#define IS_HERB_OBJECT(ob)       IS_CREATE_OBJECT((ob), HERB_OBJECT)
#define IS_HOLDABLE_OBJECT(ob)   (ob)->query_holdable_item()
#define IS_KEY_OBJECT(ob)        IS_CREATE_OBJECT((ob), KEY_OBJECT)
#define IS_LAUNCH_OBJECT(ob)     IS_CREATE_SOME((ob), "create_weapon", LAUNCH_OBJECT)
#define IS_LEFTOVER_OBJECT(ob)   IS_CREATE_SOME((ob), "create_food", LEFTOVER_OBJECT)
#define IS_LIVING_OBJECT(ob)     IS_CREATE_CONTAINER((ob), LIVING_OBJECT)
#define IS_MOBILE_OBJECT(ob)     IS_CREATE_SOME((ob), "create_living", MOBILE_OBJECT)
#define IS_MONSTER_OBJECT(ob)    IS_CREATE_SOME((ob), "create_npc", MONSTER_OBJECT)
#define IS_OBJECT_OBJECT(ob)     IS_CREATE_SOME((ob), "create", OBJECT_OBJECT)
#define IS_PARALYZE_OBJECT(ob)   IS_CREATE_OBJECT((ob), PARALYZE_OBJECT)
#define IS_PLAYER_OBJECT(ob)     IS_CREATE_SOME((ob), "create_living", PLAYER_SEC_OBJECT)
#define IS_POISON_OBJECT(ob)     IS_CREATE_OBJECT((ob), POISON_OBJECT)
#define IS_POTION_OBJECT(ob)     IS_CREATE_OBJECT((ob), POTION_OBJECT)
#define IS_PROJECTILE_OBJECT(ob) IS_CREATE_HEAP((ob), PROJECTILE_OBJECT)
#define IS_RECEPTACLE_OBJECT(ob) IS_CREATE_CONTAINER((ob), RECEPTACLE_OBJECT)
#define IS_RESISTANCE_OBJECT(ob) IS_CREATE_OBJECT((ob), RESISTANCE_OBJECT)
#define IS_ROOM_OBJECT(ob)       IS_CREATE_CONTAINER((ob), ROOM_OBJECT)
#define IS_ROPE_OBJECT(ob)       IS_CREATE_OBJECT((ob), ROPE_OBJECT)
#define IS_SCROLL_OBJECT(ob)     IS_CREATE_OBJECT((ob), SCROLL_OBJECT)
#define IS_SPELLS_OBJECT(ob)     IS_CREATE_OBJECT((ob), SPELLS_OBJECT)
#define IS_TORCH_OBJECT(ob)      IS_CREATE_OBJECT((ob), TORCH_OBJECT)
#define IS_WEARABLE_OBJECT(ob)   (ob)->query_wearable_item()
#define IS_WEAPON_OBJECT(ob)     IS_CREATE_OBJECT((ob), WEAPON_OBJECT)
#define IS_WORKROOM_OBJECT(ob)   IS_CREATE_SOME((ob), "create_room", WORKROOM_OBJECT)

/* Filter objects to be based on a certain constructor. */
#define FILTER_CREATE_SOME(obs, func, file) filter((obs), (: file == function_exists(func, $1) :) )
#define FILTER_CREATE_CONTAINER(obs, file) FILTER_CREATE_SOME((obs), "create_container", (file))
#define FILTER_CREATE_OBJECT(obs, file)    FILTER_CREATE_SOME((obs), "create_object", (file))
#define FILTER_CREATE_HEAP(obs, file)      FILTER_CREATE_SOME((obs), "create_heap", (file))

#define FILTER_ARMOUR_OBJECTS(obs)     FILTER_CREATE_OBJECT((obs), ARMOUR_OBJECT)
#define FILTER_ARROW_OBJECTS(obs)      FILTER_CREATE_SOME((obs), "create_projectile", ARROW_OBJECT)
#define FILTER_BOARD_OBJECTS(obs)      FILTER_CREATE_OBJECT((obs), BOARD_OBJECT)
#define FILTER_BOOK_OBJECTS(obs)       FILTER_CREATE_OBJECT((obs), BOOK_OBJECT)
#define FILTER_BOW_OBJECTS(obs)        FILTER_CREATE_SOME((obs), "create_launch_weapon", BOW_OBJECT)
#define FILTER_COINS_OBJECTS(obs)      FILTER_CREATE_HEAP((obs), COINS_OBJECT)
#define FILTER_CONTAINER_OBJECTS(obs)  FILTER_CREATE_OBJECT((obs), CONTAINER_OBJECT)
#define FILTER_CORPSE_OBJECTS(obs)     FILTER_CREATE_CONTAINER((obs), CORPSE_OBJECT)
#define FILTER_CREATURE_OBJECTS(obs)   FILTER_CREATE_SOME((obs), "create_mobile", CREATURE_OBJECT)
#define FILTER_DOOR_OBJECTS(obs)       FILTER_CREATE_OBJECT((obs), DOOR_OBJECT)
#define FILTER_DRINK_OBJECTS(obs)      FILTER_CREATE_HEAP((obs), DRINK_OBJECT)
#define FILTER_FOOD_OBJECTS(obs)       FILTER_CREATE_HEAP((obs), FOOD_OBJECT)
#define FILTER_HEAP_OBJECTS(obs)       FILTER_CREATE_OBJECT((obs), HEAP_OBJECT)
#define FILTER_HERB_OBJECTS(obs)       FILTER_CREATE_OBJECT((obs), HERB_OBJECT)
#define FILTER_HOLDABLE_OBJECTS(obs)   filter((obs), &->query_holdable_item())
#define FILTER_KEY_OBJECTS(obs)        FILTER_CREATE_OBJECT((obs), KEY_OBJECT)
#define FILTER_LAUNCH_OBJECTS(obs)     FILTER_CREATE_SOME((obs), "create_weapon", LAUNCH_OBJECT)
#define FILTER_LEFTOVER_OBJECTS(obs)   FILTER_CREATE_SOME((obs), "create_food", LEFTOVER_OBJECT)
#define FILTER_LIVING_OBJECTS(obs)     FILTER_CREATE_CONTAINER((obs), LIVING_OBJECT)
#define FILTER_MOBILE_OBJECTS(obs)     FILTER_CREATE_SOME((obs), "create_living", MOBILE_OBJECT)
#define FILTER_MONSTER_OBJECTS(obs)    FILTER_CREATE_SOME((obs), "create_npc", MONSTER_OBJECT)
#define FILTER_OBJECT_OBJECTS(obs)     FILTER_CREATE_SOME((obs), "create", OBJECT_OBJECT)
#define FILTER_PARALYZE_OBJECTS(obs)   FILTER_CREATE_OBJECT((obs), PARALYZE_OBJECT)
#define FILTER_PLAYER_OBJECTS(obs)     FILTER_CREATE_SOME((obs), "create_living", PLAYER_SEC_OBJECT)
#define FILTER_POISON_OBJECTS(obs)     FILTER_CREATE_OBJECT((obs), POISON_OBJECT)
#define FILTER_POTION_OBJECTS(obs)     FILTER_CREATE_OBJECT((obs), POTION_OBJECT)
#define FILTER_PROJECTILE_OBJECTS(obs) FILTER_CREATE_HEAP((obs), PROJECTILE_OBJECT)
#define FILTER_RECEPTACLE_OBJECTS(obs) FILTER_CREATE_CONTAINER((obs), RECEPTACLE_OBJECT)
#define FILTER_RESISTANCE_OBJECTS(obs) FILTER_CREATE_OBJECT((obs), RESISTANCE_OBJECT)
#define FILTER_ROOM_OBJECTS(obs)       FILTER_CREATE_CONTAINER((obs), ROOM_OBJECT)
#define FILTER_ROPE_OBJECTS(obs)       FILTER_CREATE_OBJECT((obs), ROPE_OBJECT)
#define FILTER_SCROLL_OBJECTS(obs)     FILTER_CREATE_OBJECT((obs), SCROLL_OBJECT)
#define FILTER_SPELLS_OBJECTS(obs)     FILTER_CREATE_OBJECT((obs), SPELLS_OBJECT)
#define FILTER_TORCH_OBJECTS(obs)      FILTER_CREATE_OBJECT((obs), TORCH_OBJECT)
#define FILTER_WEAPON_OBJECTS(obs)     FILTER_CREATE_OBJECT((obs), WEAPON_OBJECT)
#define FILTER_WEARABLE_OBJECTS(obs)   filter((obs), (: ({int}) $1->query_wearable_item() :))
#define FILTER_WORKROOM_OBJECTS(obs)   FILTER_CREATE_SOME((obs), "create_room", WORKROOM_OBJECT)

/* No definitions beyond this line. */
#endif
