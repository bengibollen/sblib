/*
 * /secure/simul_efun.c
 *
 * This file holds all the simulated efuns. These are called as if they were
 * inherited in all objects.
 *
 * It is managed with a hidden call_other() though so the functions will
 * not appear as true internal functions in objects.
 */

/* We must use the absolute path here because SECURITY is not loaded yet
 * when we load this module.
*/
#include <std.h>
#include <commands.h>
#include <configuration.h>
#include <libfiles.h>

#include <object_info.h>

#pragma no_clone
#pragma no_inherit
#pragma strict_types

#define BACKBONE_WIZINFO_SIZE 5
#define LIVING_NAME 3
#define NAME_LIVING 4

/* Prototypes */
varargs string getuid(object ob);
int seteuid(string str);
private object logger = load_object("/lib/log");

nomask varargs void dump_array(mixed a, string tab);
nomask varargs void dump_mapping(mapping m, string tab);
static nomask void dump_elem(mixed sak, string tab);
void log_debug(string message, varargs mixed args);
void log_error(string message, varargs mixed args);
void log_info(string message, varargs mixed args);
void log_warning(string message, varargs mixed args);


/*
 * No one is allowed to shadow the simulated efuns
 */
int query_prevent_shadow()
{
    return 1;
}

/************************************************************************
 *
 * EFUN SHELLS
 *
 * These are here for reasons of compatibility and comfort
 *
 */

 /*
void
write(mixed data) = "write";
*/

void write_vbfc(mixed data)
{
    this_player()->catch_vbfc(data, 0);
}


void tell_object(object liveob, string message)
{
    liveob->catch_tell(message);
}


static string getnames(object ob)
{
    return ({string}) ob->query_real_name();
}


static string slice_cmds(mixed *ar)
{
    return ar[0];
}


/*
 * Function name: get_localcmd
 * Description:   Returns an array of commands (excluding soul commands)
 *                available to an object.
 * Arguments:     mixed ob - the object for which you want a command list
 * Returns:       string * - an array of command names
 */
varargs string *get_localcmd(mixed ob = previous_object())
{
    if (!objectp(ob))
        return ({});

    return map(query_actions(ob, QA_VERB), #'slice_cmds);
}


void localcmd()
{
    ({string}) this_player()->catch_tell(implode(get_localcmd(previous_object()), " ") + "\n");
}


int cat(string file, varargs mixed *argv)
{
    string *lines;
    int i;
    string euid, slask;

    log_debug("Printing file: %s", file);

    sscanf(geteuid(previous_object()), "%s:%s", slask, euid);

    configure_object(this_object(), OC_EUID, "#:" + euid);

    switch(sizeof(argv))
    {
    case 0:
        slask ="" + read_file(file);
        break;
    case 1:
        slask = "" + read_file(file, argv[0]);
        break;
    case 2:
        slask = "" + read_file(file, argv[0], argv[1]);
        break;
    default:
        configure_object(this_object(), OC_EUID, "#:" + "root");
        throw("Too many arguments to cat.");
        break;
    }
    configure_object(this_object(), OC_EUID, "#:" + "root");
    lines = explode(slask, "\n");
    this_player()->catch_tell(slask);
    // Kludge warning!!!
    return (sizeof(lines) == 1 && sizeof(lines[0]) <= 3) ? 0 : sizeof(lines);
}

#define TAIL_MAX_BYTES 1000
varargs int tail(string file)
{
    if (extern_call())
        set_this_object(previous_object());

    if (!stringp(file) || !this_player())
        return 0;
    string txt = read_file(file, -(TAIL_MAX_BYTES + 80), (TAIL_MAX_BYTES + 80));
    if (!stringp(txt))
        return 0;

    // cut off first (incomplete) line
    int index = strstr(txt, "\n");
    if (index > -1) {
        if (index + 1 < sizeof(txt))
            txt = txt[index+1..];
        else
            txt = "";
    }

    tell_object(this_player(), txt);

    return 1;
}


varargs mixed *filter_array(mixed arr, mixed func, mixed ob, mixed ex)
{
    return filter(arr, func, ob, ex);
}


varargs mixed *map_array(mixed arr, mixed func, mixed ob, mixed ex)
{
    return map(arr, func, ob, ex);
}


/*
 * Function name: exclude_array
 * Description:   Deletes a section of an array
 * Arguments:     arr: The array
 *		  from: Index from which to delete elements
 *		  to: Last index to be deleted.
 * Returns:
 */
public mixed exclude_array(mixed arr, int from, int to)
{
    mixed a,b;
    if (!pointerp(arr))
	return 0;

    if (from > sizeof(arr))
	from = sizeof(arr);
    a = (from <= 0 ? ({}) : arr[0 .. from - 1]);

    if (to < 0)
	to = 0;
    b = (to >= sizeof(arr) - 1 ? ({}) : arr[to + 1 .. sizeof(arr) - 1]);

    return a + b;
}


/*
 * Function name: inventory
 * Description:   Returns a specified item in the inventory of an object
 * Arguments:     mixed ob  - the object from which to get the item
 *                mixed num - The item to select.  This should be an
 *                            integer which is used as the index to
 *                            the array returned by all_inventory(ob)
 * Returns:       object -    the 'num'-th item in the object's inventory
 */
varargs nomask object inventory(mixed ob, mixed num)
{
    object         *inv;

    if (!objectp(ob))
    {
	if (intp(ob))
	    num = ob;
	ob = previous_object();
    }

    inv = all_inventory(ob);

    if ((num <= sizeof(inv)) && (num >= 0))
	return inv[num];
    else
	return 0;
}


/*
 * Function name: all_environment
 * Description:   Gives an array of all containers which an object is in, i.e.
 *		  match in matchbox in bigbox in chest in room, would for the
 *		  match give: matchbox, bigbox, chest, room
 * Arguments:     ob: The object
 * Returns:       The array of containers.
 */
nomask object *all_environment(object ob)
{
    object         *r;

    if (!ob || !environment(ob))
	return 0;
    if (!environment(environment(ob)))
	return ({ environment(ob)  });

    r = ({ ob = environment(ob) });

    while (environment(ob))
	r = r + ({ ob = environment(ob)  });
    return r;
}


/*
 * query_xverb should return the part of the verb that had to be filled in
 * when an add_action(xxx, "yyyyy", 1) was executed.
 */
nomask string query_xverb()
{
    int size = sizeof(query_verb(1));

    return query_verb()[size..];
}


/*
 * Function name: one_of_list
 * Description  : Randomly picks one element from an array. When the argument
 *                is not an array, or is an empty array, 0 is returned.
 * Arguments    : mixed array - an array of elements of some sorts.
 *                int seed - an optional argument with the seed to use.
 * Returns      : mixed - one element from the array or 0.
 */
varargs mixed one_of_list(mixed array, int seed)
{
    if (!pointerp(array))
    {
        return 0;
    }
    if (!sizeof(array))
    {
        return 0;
    }

    return array[random(sizeof(array))];
}


/*
 *
 * These are the LPC backwardscompatible simulated efuns.
 *
 */

/*
 * Function name: export_uid
 * Description:   Sets the euid of one object to the uid of previous_object().
 *                This can only be done if the euid of the object is 0.
 * Arguments:     object ob - the object to set the euid for
 */
void export_uid(object ob)
{
    string euid, euid2;
    euid = explode(geteuid(ob),":")[1];
    euid2 = explode(geteuid(previous_object()), ":")[1];
    if (euid2 == "0")
	throw("Illegal to export euid 0");
    if (euid == "0")
        configure_object(ob, OC_EUID, euid2 + ":#");
}


/*
 * Function name: strchar
 * Description:   Convert an integer from 0 to 255 to its corresponding
 *                letter.
 * Arguments:     int x - the integer to convert
 * Returns:       string - the corresponding letter.
 */
public string strchar(int x)
{
    if (x < 0 || x > 255)
        return " ";
    
    return to_string(({x}));
}


/*
 * Function name: type_name
 * Description  : This function will return the type of a variable in string
 *                form.
 * Arguments    : mixed etwas - the variable to type.
 * Returns      : string - the name of the type.
 */
static string type_name(mixed etwas)
{
    if (intp(etwas))
	return "int";
    else if (floatp(etwas))
	return "float";
    else if (stringp(etwas))
	return "string";
    else if (objectp(etwas))
	return "object";
    else if (pointerp(etwas))
	return "array";
    else if (mappingp(etwas))
	return "mapping";
#ifdef _FUNCTION
    else if (functionp(etwas))
	return "function";
#endif
    return "!UNKNOWN!";
}


/*
 * Function name: dump_array
 * Description:   Dumps a variable with write() for debugging purposes.
 * Arguments:     a: Anything including an array
 */
nomask varargs void dump_array(mixed a, string tab)
{
    int             n,
                    m;
    mixed 	    ix, val;

    if (!tab)
	tab = "";
    if (!pointerp(a) && !mappingp(a))
    {
	dump_elem(a, tab);
	return;
    }
    else if (pointerp(a))
    {
	this_player()->catch_tell("(Array)\n");
	m = sizeof(a);
	n = 0;
	while (n < m)
	{
	    this_player()->catch_tell(tab + "[" + n + "] = ");
	    dump_elem(a[n], tab);
	    n += 1;
	}
    }
    else /* Mappingp */
	dump_mapping(a, tab);
}


/*
 * Function name: dump_mapping
 * Description:   Dumps a variable with write() for debugging purposes.
 * Arguments:     a: Anything including an array
 */
nomask varargs void dump_mapping(mapping m, string tab)
{
    mixed *d;
    int i, s;
    string dval, val;

    if (!tab)
	tab = "";

    d = sort_array(m_indices(m), #'>);
    s = sizeof(d);
    this_player()->catch_tell("(Mapping) ([\n");
    for(i = 0; i < s; i++) {
	if (intp(d[i]))
	    dval = "(int)" + d[i];

	if (floatp(d[i]))
	    dval = "(float)" + to_float(d[i]);

	if (stringp(d[i]))
	    dval = "\"" + d[i] + "\"";

	if (objectp(d[i]))
	    dval = object_name(d[i]);

	if (pointerp(d[i]))
	    dval = "(array:" + sizeof(d[i]) + ")";

	if (mappingp(d[i]))
	    dval = "(mapping:" + sizeof(d[i]) + ")";
#ifdef _FUNCTION
	if (functionp(d[i]))
	    dval = sprintf("%O", d[i]);

	if (functionp(m[d[i]]))
	    val = sprintf("%O", m[d[i]]);
#endif

	if (intp(m[d[i]]))
	    val = "(int)" + m[d[i]];

	if (floatp(m[d[i]]))
	    val = "(float)" + to_float(m[d[i]]);

	if (stringp(m[d[i]]))
	    val = "\"" + m[d[i]] + "\"";

	if (objectp(m[d[i]]))
	    val = object_name(m[d[i]]);

	if (pointerp(m[d[i]]))
	    val = "(array:" + sizeof(m[d[i]]) + ")";

	if (mappingp(m[d[i]]))
	    val = "(mapping:" + sizeof(m[d[i]]) + ")";

	this_player()->catch_tell(tab + dval + ":" + val + "\n");

	if (pointerp(d[i]))
	    dump_array(d[i]);

	if (pointerp(m[d[i]]))
	    dump_array(m[d[i]]);

	if (mappingp(d[i]))
	    dump_mapping(d[i], tab + "   ");

	if (mappingp(m[d[i]]))
	    dump_mapping(m[d[i]], tab + "   ");
    }
    this_player()->catch_tell("])\n");
}


/*
 * Function name: inherit_list
 * Description:   Returns the inherit list of an object.
 * Arguments:     ob - the object to list.
 */
nomask string *inherit_list(object ob)
{
    return ({mixed}) SECURITY->do_debug("inherit_list", ob);
}


static nomask void dump_elem(mixed sak, string tab)
{
    if (pointerp(sak))
    {
	dump_array(sak, tab + "   ");
    }
    else if (mappingp(sak))
    {
	dump_mapping(sak, tab + "   ");
    }
    else
    {
	this_player()->catch_tell("(" + type_name(sak) + ") ");
	if (objectp(sak))
	    this_player()->catch_tell(object_name(sak));
	else if (floatp(sak))
	    this_player()->catch_tell(to_string(sak));
	else
	    this_player()->catch_tell(sprintf("%O",sak));
    }
    this_player()->catch_tell("\n");
}


/*
 * Function:    secure_var
 * Description: Return a secure copy of the given variable
 * Arguments:   var - the variable
 * Returns:     the secured copy
 */
mixed secure_var(mixed var)
{
    if (pointerp(var) || mappingp(var))
    {
    	return map(var, #'secure_var);
    }
    return var;
}


/*
 * Function name: composite_util
 * Description  : Returns the composition of function f applied to the result
 *                of function g applied on variable x.
 * Arguments    : function f - the outer function in the composition.
 *                function g - the inner function in the composition.
 *                mixed x    - the argument to perform the functions on.
 * Returns      : mixed - the result of the composition.
 */
// static mixed compose_util(function f, function g, mixed x)
// {
//     return f(g(x));
// }


/*
 * Function name: compose
 * Description:   The returns the composition of two functions
 *                (in the mathematical sense).  So if
 *                h = compose(f,g)  then  h(x) == f(g(x))
 * Arguments:     f, g: the functions to compose.
 * Returns:       a function which is the composition of f and g.
 */
// function compose(function f, function g)
// {
//     return &compose_util(f,g);
// }


/*
 * Function name: not
 * Description:   returns the locigal inverse of the argument
 * Arguments:     x
 * Returns:       !x
 */
int not(mixed x)
{
    return !x;
}


/*
 * Function name: minmax
 * Description  : Makes sure a value is within a certain range of a minimum
 *                and defined maximum. The routine works on integers as well
 *                as floats.
 * Arguments    : mixed value - the value to test.
 *                mixed mininum - the minimal value acceptable.
 *                mixed maximum - the maximal value acceptable.
 * Returns      : mixed - a value that adheres to: minimum <= value <= maximum
 */
mixed minmax(mixed value, mixed minimum, mixed maximum)
{
    if (value < minimum)
    {
        return minimum;
    }
    if (value > maximum)
    {
        return maximum;
    }
    return value;
}


/*
 * Strip spaces and newlines from both ends of a string.
 *
 * NB! Obvious candidate for driver efun implementation later.
 *
 * Arguments: str - the string to strip
 * Returns:   The stripped string.
 */
nomask string strip(string str)
{
    int start, end;

    start = 0;
    end = sizeof(str);
    while (start < end && (str[start] == ' ' || str[start] == '\n'))
	start++;

    end--; // Point _inside_ the string
    while (end > start && (str[end] == ' ' || str[end] == '\n'))
	end--;

    return str[start..end];
}


int file_time(string path)
{
    mixed *v;
    set_this_object(previous_object());
    return (sizeof(v = get_dir(path, GETDIR_DATES)) ? v[0] : 0);
}


/*
 * Function name: log_file
 * Description:   Logs a message in the creators ~/log/ subdir in a given
 *		  file.
 *
 * 		  The optional argument 'csize' controls the cycling size
 * 		  of the log. If the argument is == 0 the default cycle
 * 		  size is used, if the argument is == -1 the maximum cycle
 * 		  size is used, if a positive integer is given, that cycle
 * 		  size is used provided that it is less or equal to the
 * 		  maximum cycle size.
 *
 * Arguments:     file: The filename.
 *		  text: The text to add to the file
 * 		  csize: The cycle size, if any (optional)
 */
public varargs void log_file(string file, string text, int csize)
{
     mixed           path,
                     oldeuid,
                     cr;
     string 	    *split, fnam;
     int 	    il, fsize, msize, dsize;
 
     cr = ({string}) SECURITY->creator_object(previous_object());
 
     /* Let backbone objects log things in /log */
     if (cr == ({string}) SECURITY->get_bb_uid())
     {
     path = "/log";
     cr = ({string}) SECURITY->get_root_uid();
     }
     else
     {
     path = ({string}) SECURITY->query_wiz_path(cr) + "/log";
     }
 
     /* We swap to the userid of the user trying to do log_file */
     oldeuid = geteuid(this_object());
     configure_object(this_object(), OC_EUID, cr);
 
     if (file_size(path) != -2)
     {
     /* We must create the path
         */
     split = explode(path + "/", "/");
 
     for (fnam = "", il = 0; il < sizeof(split); il++)
     {
         fnam += "/" + split[il];
         if (file_size(fnam) == -1)
         mkdir(fnam);
         else if (file_size(fnam) > 0)
         {
         this_object()->seteuid(oldeuid);
         return;
         }
     }
     }
 
     file = path + "/" + file;
 
#ifdef CYCLIC_LOG_SIZE
 
     fsize = file_size(file);
     msize = CYCLIC_LOG_SIZE[cr];
     dsize = CYCLIC_LOG_SIZE[0];
 
     if (csize > 0)
     msize = ((csize > msize) ? (msize ? msize : csize) : csize);
 
     if (csize == 0)
     msize = (msize ? msize : dsize);
 
     if (csize < 0)
     msize = (msize == 0 ? dsize : msize);
 
     if (msize > 0 && fsize > msize)
     rename(file, file + ".old");
 
#endif /* CYCLIC_LOG_SIZE */
 
     write_file(file, text);
     this_object()->seteuid(oldeuid);
 
 }




 
 
 private mapping living_name_m;
 private mapping name_living_m;
 
 mapping get_livings() { return living_name_m; }
 mapping get_names() { return name_living_m; }
 
 void start_simul_efun() {

     mixed *info;
 
     if ( !(info = get_extra_wizinfo(0)) )
     set_extra_wizinfo(0, info = allocate(BACKBONE_WIZINFO_SIZE));
     if (!(living_name_m = info[LIVING_NAME]))
     living_name_m = info[LIVING_NAME] = m_allocate(0, 1);
     if (!(name_living_m = info[NAME_LIVING]))
     name_living_m = info[NAME_LIVING] = m_allocate(0, 1);
     if (find_call_out("clean_simul_efun") < 0) {
         efun::call_out("clean_simul_efun", 1800);
     }
 }
 
 //---------------------------------------------------------------------------
 static void clean_simul_efun()
 {
     /* There might be destructed objects as keys. */
     m_indices(living_name_m);
     remove_call_out("clean_simul_efun");
     if (find_call_out("clean_name_living_m") < 0)
     {
     call_out(
       "clean_name_living_m",
       1,
       m_indices(name_living_m),
       sizeof(name_living_m)
     );
     }
     call_out("clean_simul_efun", 3600);
 }
 
 //---------------------------------------------------------------------------
 static void clean_name_living_m(string *keys, int left)
 {
     int i, j;
     mixed a;
 
     if (left) {
     if (pointerp(a = name_living_m[keys[--left]]) && member(a, 0)>= 0) {
         i = sizeof(a);
         do {
         if (a[--i])
             a[<++j] = a[i];
         } while (i);
         name_living_m[keys[left]] = a = j > 1 ? a[<j..] : a[<1];
     }
     if (!a)
         efun::m_delete(name_living_m, keys[left]);
     call_out("clean_name_living_m", 1, keys, left);
     }
 }
 
 //---------------------------------------------------------------------------
 public void set_living_name(string name)
 {
     string old;
     mixed a;
     int i;
 
     if (old = living_name_m[previous_object()]) {
     if (pointerp(a = name_living_m[old])) {
         a[member(a, previous_object())] = 0;
     } else {
         efun::m_delete(name_living_m, old);
     }
     }
     living_name_m[previous_object()] = name;
     if (a = name_living_m[name]) {
     if (!pointerp(a)) {
         name_living_m[name] = ({a, previous_object()});
         return;
     }
     /* Try to reallocate entry from destructed object */
     if ((i = member(a, 0)) >= 0) {
         a[i] = previous_object();
         return;
     }
     name_living_m[name] = a + ({previous_object()});
     return;
     }
     name_living_m[name] = previous_object();
 }
 
 //---------------------------------------------------------------------------
 object find_living(string name)
 {
     mixed *a, r;
     int i;
     
     if (pointerp(r = name_living_m[name])) {
     if ( !living(r = (a = r)[0])) {
         for (i = sizeof(a); --i;) {
         if (living(a[<i])) {
             r = a[<i];
             a[<i] = a[0];
             return a[0] = r;
         }
         }
     }
     return r;
     }
     return living(r) && r;
 }
 
 //---------------------------------------------------------------------------
 
 object find_player(string name)
 {
     mixed *a, r;
     int i;
 
     if (pointerp(r=name_living_m[name])) {
     if ( !(r=(a=r)[0]) || !object_info(r, OI_ONCE_INTERACTIVE)) {
         for (i=sizeof(a); --i;) {
         if (a[<i] && object_info(a[<i], OI_ONCE_INTERACTIVE)) {
             r=a[<i];
             a[<i]=a[0];
             return a[0]=r;
         }
         }
         return 0;
     }
     return r;
     }
     return r && object_info(r, OI_ONCE_INTERACTIVE) && r;
 }


 void log_error(string message, varargs mixed args)
 {
    logger->error(message, args...);
 }


 void log_warn(string message, varargs mixed args)
 {
     logger->warn(message, args...);
 }


 void log_info(string message, varargs mixed args)
{
    logger->info(message, args...);
}


void log_debug(string message, varargs mixed args)
{
    logger->debug(message, args...);
}


void log_trace(string message, varargs mixed args)
{
    logger->trace(message, args...);
}
