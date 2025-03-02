public void setup_filesystem() {
    configure_driver(DC_FILESYSTEM_ENCODING, "UTF-8");
}

public void setup_include_dirs() {
    set_driver_hook(H_INCLUDE_DIRS, ({
        "/inc/",         // mudlib includes
        "/sys/",     // system includes
    }));
}

public void setup_error_handling() {
    // Set default error message for unknown commands
    set_driver_hook(H_NOTIFY_FAIL, "What?\n");
}

public void setup_uid_management() {
    set_driver_hook(H_LOAD_UIDS, #'load_uid);
    set_driver_hook(H_CLONE_UIDS, #'clone_uid);
    set_driver_hook(H_CREATE_SUPER, "create");
    set_driver_hook(H_CREATE_OB, "create");
    set_driver_hook(H_CREATE_CLONE, "create");
    set_driver_hook(H_TELNET_NEG, "got_telnet");
}

public void setup_all() {
    setup_filesystem();
    setup_include_dirs();
    setup_error_handling();
    setup_uid_management();
}
