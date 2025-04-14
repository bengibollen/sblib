void inaugurate_master(int arg)
{ 
    string err;

    debug_message("Master Initialization Starting\n");
    debug_message(sprintf("Master object loaded (arg: %d)\n", arg));
    
    setup_all();
    create();
    if (arg)
        reset_master();
    
    // Now it's safe to initialize the logger
    if (err = catch(logger = load_object("/lib/log"))) {
        debug_message("Failed to initialize logger: " + err + "\n");
    }
    else {
        logger->info("Logger system initialized");
    }

    logger->info("Retrieving master data.");
    logger->debug("Check file: %d\n", file_size(SAVEFILE + ".o"));
    if ((file_size(SAVEFILE + ".o") <= 0) || !restore_object(SAVEFILE))
    {
        logger->info(SAVEFILE + " nonexistant. Using defaults.");
        load_fob_defaults();
        load_guild_defaults();
        logger->debug("This object: %O", this_object());
        logger->debug("Master info: %s", save_value(this_object()));
        // reset_graph();
        runlevel = WIZ_MORTAL;
        save_master();
    }

    logger->info("=== Master Initialization Complete ===\n");
}


/*
 * Function name: start_boot
 * Description:   Loads master data, including list of all domains and wizards,
 *                then makes a list of preload stuff
 * Arguments:     load_empty: If true start_boot() does no preloading
 * Returns:       List of files to preload
 */
#if 0
static string *start_boot(int load_empty)
{
    string *prefiles = ({ });  // Initialize to empty array
    string *links;
    object simf;
    int size;

    if (game_started)
        return 0;

    configure_object(this_object(), OC_EUID, ROOT_UID);

    /* Set to reasonable defaults, if they don't exist. */
    if (!pointerp(def_locations))
    {
        def_locations = ({ });
    }
    if (!pointerp(temp_locations))
    {
        temp_locations = ({ });
    }

    /* Update some internal data. */
    update_guild_cache();
    init_sitebans();
    init_player_info();

    if (load_empty)
    {
        write("Not preloading.\n");
        return 0;
    }

#ifdef PRELOAD_FIRST
    /* Process preload files list */
    if (stringp(PRELOAD_FIRST) && (file_size(PRELOAD_FIRST) > 1))
    {
        prefiles = explode(read_file(PRELOAD_FIRST), "\n");
    }
        else if (pointerp(PRELOAD_FIRST))
    {
        prefiles = ({ PRELOAD_FIRST });
    }
#endif

    write("Loading and setting up domain links:\n");
    links = filter(query_domain_links() + query_mage_links(),
        #'load_domain_link);

    size = sizeof(links);
    while (size--)
    {
        prefiles += links[size]->query_preload();
    }

    return prefiles;
}
#endif
