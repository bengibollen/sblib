/*
 * /std/heap.c
 *
 * This is a heap object for things like coins, matches and such stuff.
 */


#pragma strict_types

inherit "/std/object";

#include <cmdparse.h>
#include <language.h>
#include <macros.h>
#include <ss_types.h>
#include <stdproperties.h>
#include <configuration.h>

/*
 * Protoypes.
 */
void count_up(int delay, int amount, int counted);
public int count(string str);
varargs int stop(string str);
public int heap_volume();
public int heap_weight();
public int heap_value();
public int heap_light();
int no_garbage_collection();
void collect_garbage();

static         int     item_count,   /* Number of items in the heap */
                       leave_behind; /* Number of items to leave behind */
static private int     count_alarm,
                       gNo_merge;
static private mapping gOwn_props;

#define GARBAGE_COLLECTION_DELAY (5 /* seconds */)

/*
 * Function name: create_object
 * Description  : Constructor. To define your own heaps, use create_heap().
 */
nomask void
create_object()
{
    add_prop(OBJ_I_WEIGHT, #'heap_weight);
    add_prop(OBJ_I_VOLUME, #'heap_volume);
    add_prop(OBJ_I_VALUE,  #'heap_value);
    add_prop(OBJ_I_LIGHT,  #'heap_light);
    add_prop(HEAP_I_IS, 1);

    gOwn_props = obj_props;

    if (!no_garbage_collection())
    {
        call_out(#'collect_garbage, GARBAGE_COLLECTION_DELAY);
    }

    this_object()->create_heap();
}

/*
 * Function name: reset_object
 * Description  : Resets this objects. Define reset_heap() to reset the heap.
 */
public nomask void
reset_object()
{
    this_object()->reset_heap();
}

/*
 * Function name: init
 * Description  : This function is called each time the object 'meets' another
 *                object.
 */
void
init()
{
    ::init();
    add_action(#'count, "count");
}

/*
 * Function name: heap_weight
 * Description  : The weight of the heap, used by OBJ_I_WEIGHT
 * Returns      : int - the weight of the heap, in grams.
 */
public int
heap_weight()
{
    return query_prop(HEAP_I_UNIT_WEIGHT) * (item_count - leave_behind);
}

/*
 * Function name: heap_volume
 * Description  : The volume of the heap, used by OBJ_I_VOLUME
 * Returns      : int - the volume of the heap, in ml.
 */
public int
heap_volume()
{
    return query_prop(HEAP_I_UNIT_VOLUME) * (item_count - leave_behind);
}

/*
 * Function name: heap_value
 * Description  : The value of the heap, used by OBJ_I_VALUE
 * Returns      : int - the value of the heap.
 */
public int
heap_value()
{
    return query_prop(HEAP_I_UNIT_VALUE) * (item_count - leave_behind);
}

/*
 * Function name: heap_light
 * Description  : The light emitted by the heap, used by OBJ_I_LIGHT
 * Returns      : int - the light of the heap.
 */
public int
heap_light()
{
    return query_prop(HEAP_I_UNIT_LIGHT) * (item_count - leave_behind);
}

/*
 * Function name: restore_heap
 * Description  : Called to restore the heap to its origional state after
 *                preparations were made to split it were made with
 *                split_heap().
 */
public void
restore_heap()
{
    if (leave_behind)
    {
        mark_state();
        leave_behind = 0;
        update_state();
    }
}

/*
 * Function name: set_heap_size
 * Description  : Set the size of the heap to num.
 * Arguments    : int num - the size of the heap.
 */
public void
set_heap_size(int num)
{
    mark_state();

    if (num <= 0)
    {
        add_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT, 1);
        call_out(#'remove_object, 0);
        leave_behind = 0;
        num = 0;
    }
    item_count = num;

    /* We must update the weight and volume of what we reside in. */
    update_state();
}

/*
 * Function name: num_heap
 * Description  : Returns the size of the heap. In case preparations were
 *                made to split the heap, this will return how many items are
 *                going to be moved.
 * Returns      : int - the number of elements in the heap.
 */
public int
num_heap()
{
    return item_count - leave_behind;
}

/*
 * Function name: query_leave_behind
 * Descriptioon : Returns the size of the heap that remains after some
 *                elements have been moved away from the heap.
 * Returns      : int - the number of elements that will remain behind.
 */
public int
query_leave_behind()
{
    return leave_behind;
}

/*
 * Function name: split_heap
 * Description  : Called before a pending move of a part of the heap. This
 *                reduces the heap to the number of elements that is being
 *                moved. Afterwards the heap is restored.
 * Arguments    : int num - the number of items that is being split from the
 *                    heap in order to be moved.
 */
public int
split_heap(int num)
{
    mark_state();
    if (item_count <= num)
    {
        leave_behind = 0;
        /* It is ok to return here without updating the state as the
         * entire heap will be moved.
         */
        return item_count;
    }
    leave_behind = item_count - num;
    call_out(#'restore_heap, 0);
    update_state();
    return num;
}

/*
 * Function name: remove_split_heap
 * Description  : Whenever a heap is split in order to be moved, we can also
 *                simply destroy that part. This effectively lowers the heap
 *                with the number of elements that were going to be moved.
 */
public void
remove_split_heap()
{
    mark_state();
    item_count = leave_behind;
    leave_behind = 0;

    if (item_count <= 0)
    {
    	add_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT, 1);
        call_out(#'remove_object, 0);
    }

    update_state();
}

/*
 * Function name: singular_short
 * Description  : This function will return the singular short descritpion
 *                for this heap.
 * Arguments    : object for_obj - who wants to know.
 * Returns      : string - the singular short description.
 */
public varargs string
singular_short(object for_obj)
{
    return ::short(objectp(for_obj) ? for_obj : this_player());
}

/*
 * Function name: pluarl_short
 * Description  : This function will return the plural short descritpion
 *                for this heap.
 * Arguments    : object for_obj - who wants to know.
 * Returns      : string - the plural short description.
 */
public varargs string
plural_short(object for_obj)
{
    string str = ::plural_short(for_obj);

    if (!stringp(str))
    {
        str = singular_short(for_obj);
        str = LANG_PSENT(str);
    }

    return str;
}

/*
 * Function name: short
 * Description  : Get the short description for the heap.
 * Arguments    : object for_obj - who wants to know.
 * Returns      : string - the short description.
 */
public varargs string
short(object for_obj)
{
    string str;

    if (!sizeof(query_prop(HEAP_S_UNIQUE_ID)))
    {
        add_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT, 1);
        call_out(#'remove_object, 0);
        return "ghost " + singular_short(for_obj);
    }

    if (num_heap() < 1)
    {
        return 0;
    }
    if (num_heap() < 2)
    {
        str = singular_short(for_obj);
        return LANG_ADDART(str);
    }

    str = plural_short(for_obj);

    if (num_heap() < 12)
    {
        return LANG_WNUM(num_heap()) + " " + str;
    }
    if (num_heap() == 12)
    {
        return "a dozen " + str;
    }
    if (({int}) this_player()->query_stat(SS_INT) / 2 > num_heap())
    {
        return num_heap() + " " + str;
    }

    return (num_heap() < 1000 ? "many " : "a huge heap of ") + str;
}

/*
 * Function name: query_objective
 * Description  : All nonlivings have objective 'it'. Heaps may have 'them'
 *                if they are larger than one item.
 * Returns      : string "it" or "them".
 */
public string
query_objective()
{
    if (num_heap() > 1)
    {
        return "them";
    }
    return "it";
}

/*
 * Function name: query_possessive
 * Description  : All nonlivings have possessive 'its'. Heaps may have 'their'
 *                if they are larger than one item.
 * Returns      : string "its" or "their".
 */
public string
query_possessive()
{
    if (num_heap() > 1)
    {
        return "their";
    }
    return "its";
}

/*
 * Function name: query_pronoun
 * Description  : All nonlivings have pronoun 'it'. Heaps may have 'they' if
 *                they are larger than one item.
 * Returns      : string "it" or "they".
 */
public string
query_pronoun()
{
    if (num_heap() > 1)
    {
        return "they";
    }
    return "it";
}

/*
 * Function name: make_leftover_heap
 * Description:   clone a heap to be used as the leftover
 *                heap after a split
 * Returns:       The leftover heap object
 */
public object
make_leftover_heap()
{
    object ob;

    if (!leave_behind)
        return 0;

    if (item_count <= leave_behind)
        return 0;

    if (!geteuid(this_object()))
        configure_object(this_object(), OC_EUID, getuid());

    ob = CLONE_COPY;
    ob->config_split(leave_behind, this_object());
    item_count -= leave_behind;
    leave_behind = 0;

    return ob;
}

/*
 * Description: Called when heap leaves it's environment
 */
public void
leave_env(object env, object dest)
{
    object ob;

    ::leave_env(env, dest);

    if (ob = make_leftover_heap())
    {
        ob->move(env, 1);
    }
}

/*
 * Description: Called when heap enters and environment
 */
public void
enter_env(mixed env, object old)
{
    object *ob;
    int i, tmphide, tmpinvis;
    string unique;

    ::enter_env(env, old);

    if (gNo_merge)
    {
        return;
    }

    if (!objectp(env))
    {
        env = find_object(env);
    }
    ob = filter(all_inventory(env) - ({ this_object() }),
        (: ({int}) $1->query_prop(HEAP_I_IS) :) );

    tmphide = !!query_prop(OBJ_I_HIDE);
    tmpinvis = !!query_prop(OBJ_I_INVIS);
    unique = query_prop(HEAP_S_UNIQUE_ID);

    for (i = 0; i < sizeof(ob); i++)
    {
        if ((({string}) ob[i]->query_prop(HEAP_S_UNIQUE_ID) == unique) &&
            (!!({int}) ob[i]->query_prop(OBJ_I_HIDE) == tmphide) &&
            (!!({int}) ob[i]->query_prop(OBJ_I_INVIS) == tmpinvis) &&
            !({int}) ob[i]->query_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT))
        {
            ob[i]->set_heap_size(item_count + ({int}) ob[i]->num_heap());

            if (({int}) ob[i]->query_keepable() && !(({int}) ob[i]->query_keep()))
            {
                ob[i]->set_keep(({int}) this_object()->query_keep());
            }

            leave_behind = 0;
            catch(move(ob[i], 1));
            add_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT, 1);
            call_out(#'remove_object, 0);
            return;
        }
    }
}

/*
 * Function name: set_no_merge
 * Description:   Make sure that the heap won't merge
 */
public void
set_no_merge(int i)
{
    if (i && !gNo_merge)
    {
        call_out(#'set_no_merge, 0, 0);
    }

    gNo_merge = i;
}

public void
force_heap_merge()
{
    object *ob;
    int i, tmphide, tmpinvis;
    string unique;

    ob = filter(all_inventory(environment(this_object())) - ({ this_object() }),
        (: ({int}) $1->query_prop(HEAP_I_IS) :) );

    tmphide = !!query_prop(OBJ_I_HIDE);
    tmpinvis = !!query_prop(OBJ_I_INVIS);
    unique = query_prop(HEAP_S_UNIQUE_ID);

    for (i = 0; i < sizeof(ob); i++)
    {
        if ((({string}) ob[i]->query_prop(HEAP_S_UNIQUE_ID) == unique) &&
            (!!({int}) ob[i]->query_prop(OBJ_I_HIDE) == tmphide) &&
            (!!({int}) ob[i]->query_prop(OBJ_I_INVIS) == tmpinvis) &&
            !({int}) ob[i]->query_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT))
        {
            ob[i]->set_heap_size(item_count + ({int}) ob[i]->num_heap());
    	    add_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT, 1);
            call_out(#'remove_object, 0);
        }
    }
}

/*
 * Function name: force_heap_split
 * Description:   Cause the heap to split into two.
 * Returns:       The new heap
 */
public object
force_heap_split()
{
    object ob;

    if (ob = make_leftover_heap())
    {
        ob->set_no_merge(1);
        ob->move(environment(), 1);
    }

    return ob;
}

/*
 * Function name: query_prop_map
 * Description:   Returns mapping containg all props and their values.
 * Returns:       The obj_props mapping.
 */
public nomask mapping
query_prop_map()
{
    return secure_var(obj_props);
}

/*
 * Description: This is called before inserting this heap into the game
 */
void
config_split(int new_num, object orig)
{
    item_count = new_num;

    set_name(({string}) orig->query_name(1));
    remove_name(OB_NAME(orig));
    set_pname(({string}) orig->query_pname(1));
    set_adj(({string}) orig->query_adj(1));

    set_short(({string}) orig->query_short());
    set_pshort(({string}) orig->query_plural_short());
    set_long(({string}) orig->query_long());

    obj_props = ({mapping}) orig->query_prop_map() + gOwn_props;

    if (({int}) orig->query_keepable())
    {
        this_object()->set_keep(({int}) orig->query_keep());
    }
}

/*
 * Description: Function called when player gives 'count' command
 */
public int count(string str)
{
    string *tmp;
    int delay;
    int intg;

    if (!check_seen(this_player()))
    {
        return 0;
    }

    if (({int}) this_player()->query_attack())
    {
        notify_fail("You are too busy fighting to count anything!\n");
        return 0;
    }

    if (!stringp(str) ||
        !parse_command(str, ({ this_object() }), "%i", tmp))
    {
        notify_fail("Count what?\n");
        return 0;
    }

    intg = ({int}) this_player()->query_stat(SS_INT);
    delay = to_int(60 / to_float(intg));
    /* count_arg contains interval, coins per count and total so far */
    call_out(#'count_up, delay, delay, 5 * (intg / 10 + 1), 0);
    add_action(#'stop, "", 1);

    write("You start counting your " + plural_short(this_player()) + ".\n");
    say(QCTNAME(this_player()) + " starts to count some " +
        plural_short(this_player()) + ".\n");
    return 1;
}

/*
 * Description: Stop counting the items in the heap.
 */
varargs int
stop(string str)
{
    if (query_verb() == "stop")
    {
        update_actions();
        remove_call_out(#'count_up);
        write("You stop counting.\n");
        say(QCTNAME(this_player()) + " stops counting.\n");
        return 1;
    }

    /* Allow wizards and allow commands that are allowed. */
    if (({int}) this_player()->query_wiz_level() ||
    	(query_verb() in CMDPARSE_PARALYZE_ALLOWED))
    {
        /* When quitting, update the actions, so people can drop stuff. */
        if (query_verb() == "quit")
        {
            update_actions();
        }
        return 0;
    }

    write("You are busy counting. You have to stop if you want " +
        "to do something else.\n");
    return 1;
}

/*
 * Description: Count some more, how much depends on intelligence of player
 */
void count_up(int delay, int amount, int counted)
{
    counted += amount;
    if (counted < num_heap())
    {
        write("... " + counted + "\n");
        delay = to_int(delay);
        call_out(#'count_up, delay, delay, amount, counted);
    }
    else
    {
        write("The last count reached " + num_heap() + ".\n");
        say(QCTNAME(this_player()) + " finishes " +
            ({string}) this_player()->query_possessive() + " counting.\n");
        update_actions();
    }
}

/*
 * Function name: collect_garbage
 * Description  : When the heap is still in the void some time after it was
 *                cloned, we destruct it to save memory. If you really do not
 *                want this, mask no_garbage_collection() to return 1, and
 *                have a good reason to explain this when asked by an arch.
 */
void
collect_garbage()
{
    if (!environment())
    {
        remove_object();
    }
}

/*
 * Function name: no_garbage_collection
 * Description  : By default, heaps will be destructed if they stay in the
 *                void for more than a few seconds. If you have a very good
 *                reason to keep your heap in the void, mask this function
 *                to return 1.
 * Returns      : int 0 - always.
 */
int
no_garbage_collection()
{
    return 0;
}

/*
 * Function name: appraise_number
 * Description:   This function is called when someon tries to appraise number
 *                of pices in heap of this object.
 * Arguments:     num - use this number instead of skill if given.
 */
public int
appraise_number(int num)
{
    int value, skill, seed;

    if (!num)
        skill = ({int}) this_player()->query_skill(SS_APPR_OBJ);
    else
        skill = num;

    skill = 1000 / (skill + 1);
    value = num_heap();
    sscanf(OB_NUM(this_object()), "%d", seed);
    skill = random(skill);
    value = cut_sig_fig(value + (skill % 2 ? -skill % 70 : skill) *
        value / 100);

    return value;
}

/*
 * Description: Called when player tries to appraise the heap.
 */
void
appraise_object(int num)
{
    write(({string}) this_object()->long() + "\n");
    write("You appraise that the weight is " +
        appraise_weight(num) + " and you guess its volume is about " +
        appraise_volume(num) + ".\n");
    write("You estimate that there are " + appraise_number(num) +
        " pieces worth approx " + appraise_value(num) + ".\n");
}

/*
 * Function name: stat_object
 * Description:   This function is called when a wizard wants to get more
 *                information about an object.
 * Returns:       str - The string to write..
 */
string
stat_object()
{
    string str;

    str = ::stat_object();

    str += "Num: " + num_heap() + ".\n";

    return str;
}

/*
 * Function name: add_prop_obj_i_value
 * Description:   Hook to avoid wrong settings of OBJ_I_VALUE in a heap
 * Arguments:     val - The value OBJ_I_VALUE is intended to be set to
 * Returns:          1 - If OBJ_I_VALUE shouldn't get this new setting.
 */
int
add_prop_obj_i_value(mixed val)
{
    if (!closurep(val))
    {
        add_prop(HEAP_I_UNIT_VALUE, val);
        return 1;
    }

    return 0;
}

/*
 * Function name: add_prop_obj_i_volume
 * Description:   Hook to avoid wrong settings of OBJ_I_VOLUME in a heap
 * Arguments:     val - The value OBJ_I_VOLUME is intended to be set to
 * Returns:       1 - If OBJ_I_VOLUME shouldn't get this new setting.
 */
int
add_prop_obj_i_volume(mixed val)
{
    if (!closurep(val))
    {
        add_prop(HEAP_I_UNIT_VOLUME, val);
        return 1;
    }

    return 0;
}

/*
 * Function name: add_prop_obj_i_weight
 * Description:   Hook to avoid wrong settings of OBJ_I_WEIGHT in a heap
 * Arguments:     val - The value OBJ_I_WEIGHT is intended to be set to
 * Returns:       1 - If OBJ_I_WEIGHT shouldn't get this new setting.
 */
int
add_prop_obj_i_weight(mixed val)
{
    if (!closurep(val))
    {
        add_prop(HEAP_I_UNIT_WEIGHT, val);
        return 1;
    }

    return 0;
}
