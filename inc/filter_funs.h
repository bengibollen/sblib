/*
 * /sys/filter_funs.h
 *
 * A set of functions often used by filter.
 */

#ifndef FILTER_DEF
#define FILTER_DEF

/*
 * returns all livings in array x.
 */
#define FILTER_LIVE(x) \
    filter((x), #'living)

/*
 * returns all livings in array x without this_player().
 */
#define FILTER_OTHER_LIVE(x) \
    (FILTER_LIVE((x) - ({ this_player() }) ))

/*
 * returns all interactive players in array x.
 */
#define FILTER_PLAYERS(x) \
    filter((x), #'interactive)

/*
 * returns all non-living objects in array x.
 */
#define FILTER_DEAD(x) \
    filter(((x) - ({ 0 }) ), (: !living($1) :))

/*
 * returns all elements in array x that are present in the environment of
 * this_player().
 *
 * Caveat: This filter is _not_ forgiving when 0 is in the array.
 */
#define FILTER_PRESENT(x) \
    filter((x), (: environment($1) == environment(this_player()) :))

/*
 * returns all livings in array x that are present in the environment of
 * this_player().
 */
#define FILTER_PRESENT_LIVE(x) \
    filter(FILTER_LIVE(x), (: environment($1) == environment(this_player()) :))

/*
 * returns all objects in array x that are of gender g.
 */
#define FILTER_GENDER(x, g) \
    filter((x), (: ({int}) $1->query_gender() == (g) :))

/*
 * returns all objects in array x that are of race r.
 */
#define FILTER_RACE(x, r) \
    filter((x), (: ({int}) $1->query_race_name() == (r) :))

/*
 * returns all objects in array x that are a wizard.
 */
#define FILTER_IS_WIZARD(x) \
    filter((x), (: ({int}) $1->query_wiz_level() :))

/*
 * returns all objects in array x that are a mortal player,
 * or rather, that are not a wizard.
 */
#define FILTER_IS_MORTAL(x) \
    filter((x), (: !({int}) $1->query_wiz_level() :))

/*
 * returns all objects in array x that can see in the room.
 */
#define FILTER_CAN_SEE_IN_ROOM(x) \
    filter((x), (: ({int}) $1->can_see_in_room() :))

/*
 * returns all objects in array x that can be seen by living l.
 */
#define FILTER_CAN_SEE(x, l) \
    filter((x), (: ({int}) $1->check_seen((l)) :))

/*
 * returns all objects in array x that can see [living/object] l.
 */
#define FILTER_IS_SEEN(l, x) \
    filter((x), (: ({int}) (l)->check_seen($1) :))

/*
 * returns all objects in array x that can be seen, ie that aren't no_show.
 */
#define FILTER_SHOWN(x) \
    filter((x), (: !({int}) $1->query_no_show() :))

/* no definitions beyond this line. */
#endif
