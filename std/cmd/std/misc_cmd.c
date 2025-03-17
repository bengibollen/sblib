/*
 * /cmd/std/misc_cmd.c
 *
 * This item refers to the souls that are now in /cmd/live/
 */
#pragma strict_types

inherit "/std/command_driver.c";

/*
 * Function name: replace_soul
 * Description  : This soul is replaced with the following souls.
 * Returns      : string * - the souls we replace this soul with.
 */
public string *replace_soul()
{
    return
	({
//	    "/std/cmd/live/info",
	    "/std/cmd/live/items",
	    "/std/cmd/live/magic",
	    "/std/cmd/live/social",
	    "/std/cmd/live/speech",
	    "/std/cmd/live/state",
        "/std/cmd/live/thief",
	    "/std/cmd/live/things",
	});
}
