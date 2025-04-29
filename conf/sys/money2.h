/*
 * money2.h
 *
 * This file defines the local currency units.
 * The only use of this file is that it should be included by /sys/money.h.
 */

#define SIZEOF_MONEY_TYPES 4
#define MONEY_TYPES  ({ "copper", "silver", "gold", "platinum" })
#define MONEY_SHORT  ({ "cc",     "sc",     "gc",   "pc" })
#define MONEY_VALUES ({ 1,         12,       144,    1728 })
#define MONEY_WEIGHT ({ 7,         42,       100,   120 })
#define MONEY_VOLUME ({ 3,         2,        3,      4 })

#define MONEY_MAKE_COPPER(num)     MONEY_MAKE(num, "copper")
#define MONEY_MAKE_SILVER(num)     MONEY_MAKE(num, "silver")
#define MONEY_MAKE_GOLD(num)       MONEY_MAKE(num, "gold")
#define MONEY_MAKE_PLATINUM(num)   MONEY_MAKE(num, "platinum")

#define MONEY_MOVE_COPPER(num, from, to)     MONEY_MOVE("copper",   num, from, to)
#define MONEY_MOVE_SILVER(num, from, to)     MONEY_MOVE("silver",   num, from, to)
#define MONEY_MOVE_GOLD(num, from, to)       MONEY_MOVE("gold",     num, from, to)
#define MONEY_MOVE_PLATINUM(num, from, to)   MONEY_MOVE("platinum", num, from, to)

#define MONEY_LOG_LIMIT ([ "copper": 10000, "silver": 10000, "gold": 10000, "platinum": 10000 ])
