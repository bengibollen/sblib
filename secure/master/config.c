static void _move_hook_fun (object item, object dest)

// Move object <item> into object <dest>.
//
// Argument:
//   item: the object to be moved.
//   dest: the destination for the object.
//
// The function performs all the checks for a valid move (item exists,
// destination exists, destination is not current environment, etc).
// In addition, it implements the init() protocol needed for add_action()
// to work.

{
    object *others;
    int i;
    string name;

    /* PLAIN:
    if (item != this_object)
    raise_error("Illegal to move other object than this_object()\n");
    */

    if (living(item) && environment(item))
    {
        name = object_info(item, OI_ONCE_INTERACTIVE)
        ? ({string}) item->query_real_name()
        : object_name(item);
        /* PLAIN: the call to exit() is needed in compat mode only */
        efun::set_this_player(item);
        object env = environment(item);
        env->exit(item);
        if (!item)
        raise_error(sprintf("%O->exit() destructed item %s before move.\n"
                        , env, name));
    }
    else
        name = object_name(item);

    /* This is the actual move of the object. */

    efun::set_environment(item, dest);

    /* Moving a living object will cause init() to be called in the new
    * environment.
    */
    if (living(item))
    {
        efun::set_this_player(item);
        dest->init();
        if (!item)
            raise_error(sprintf("%O->init() destructed moved item %s\n", dest, name));
        if (environment(item) != dest)
            return;
    }
    /* Call init() in item once foreach living object in the new environment
    * but only if the item is (still) in the same environment.
    */
    others = all_inventory(dest) - ({ item });
    foreach (object obj : others)
    {
        if (living(obj) && environment(obj) == environment(item))
        {
            efun::set_this_player(obj);
            item->init();
        }

        if (!item)
            raise_error(sprintf("item->init() for %O destructed moved item %s\n", obj, name));
    }

    /* Call init() in each of the  objects themselves, but only if item
    * didn't move away already.
    */
    if (living(item))
    {
        foreach (object obj : others)
        {
            efun::set_this_player(item); // In case something new was cloned
            if (environment(obj) == environment(item))
                obj->init();
        }
    }

    /* If the destination is alive as well, call item->init() for it. */
    if (living(dest) && item && environment(item) == dest)
    {
        efun::set_this_player(dest);
        item->init();
    }
}

public void setup_move()
{
    set_driver_hook(
        H_MOVE_OBJECT0,
        unbound_lambda( ({'item, 'dest}),
        ({#'_move_hook_fun, 'item, 'dest })
                      )
                 );
}

public void setup_filesystem() {
    configure_driver(DC_FILESYSTEM_ENCODING, "UTF-8");
}

public void setup_include_dirs()
{
    set_driver_hook(H_INCLUDE_DIRS, ({
        "/inc/",         // mudlib includes
        "/sys/",     // system includes
    }));
}

public void setup_error_handling()
{
    // Set default error message for unknown commands
    set_driver_hook(H_NOTIFY_FAIL, "What?\n");
}

public void setup_uid_management() {
    set_driver_hook(H_LOAD_UIDS, #'load_uid);
    set_driver_hook(H_CLONE_UIDS, #'clone_uid);
    set_driver_hook(H_CREATE_SUPER, "create");
    set_driver_hook(H_CREATE_OB, "create");
    set_driver_hook(H_CREATE_CLONE, "create");
}

public void setup_all()
{
    setup_filesystem();
    setup_include_dirs();
    setup_error_handling();
    setup_uid_management();
    setup_move();
}
