
#pragma strict_types
#pragma no_clone

#include "/inc/is_creator.h"
#include "/sys/object_info.h"
#include "/inc/libfiles.h"

/*
 * check_previous_privilege() does the same as check_privilege() only
 * that it does not check the calling object's own privilege. In very
 * many cases this is very useful because this way the calling object
 * does not have to have a privilege at all.
 */
varargs int check_previous_privilege(mixed priv) {
    return ({int}) SECURITY -> check_privilege(priv, 2);
}

/* 
 * checks if the given privilege is a valid one, used by the security
 * system
 */
varargs status valid_privilege(mixed priv) {
    return ({status}) SECURITY -> valid_privilege(priv);
}

/*
 * Returns the privilege of the given object.
 */
mixed get_privilege(object ob) {
    return ({string}) function_exists("query_privilege", ob) == M_ACCESS &&
        ({int}) ob -> query_privilege();
}



/*
 * Returns the creator of the given file <name>. The creator can be
 * none(0), lib (1), a wizard, a domain
 */
mixed creator_file(string name) {
    string *path;
    
    if (strstr(name, "//") >= 0 ||
            member(name, '#') >= 0) {
        
        return 0;
    }
    
    path = explode(name, "/") - ({ "", "." });
    
    switch (sizeof(path) && path[0]) {
    case "open":
    case "log":
        return 0; 
    case "home":
        if (sizeof(path) < 3) {
            return 0;
        }
        
        // return wizards name
        return path[1];
    case "domains":
        if (sizeof(path) < 3) {
            return 0;
        }
            
        return capitalize(path[1]);
    default:
        // all other directory/files are owned by lib
        return "lib";
    }
}

/*
 * Returns the privilege which is neccessary to access the given path in the
 * given way (mode).
 * The path is either a path to a directory or to a file.
 * The mode is a string which's first character is either 'w' or 'r' to 
 * indedicate wether read or write permissions are to be queried. The
 * second charater is either 'f' or 'd' to indicate if the neccessary privilege
 * for access to a file in the given directory or to a subdirectory is wanted
 */
mixed get_protection(string file, string mode) {
    status write_priv;
    
    write_priv = 0;
    
    switch (mode[0]) {
    case 'w':
        write_priv = 1;
        
        break;
    case 'r':
        write_priv = 0;
        
        break;
    
    default:
        raise_error("Bad argument 2 (mode) to get_protection()\n");
        
        break;
    }
    
    switch (mode[1]) {
    case 'f':
        file += "foo";
        
        break;
    case 'd':
        if (file[<1]=='/') {
            file += "foo";
        }
        else {
            file += "/foo";
        }
        
        break;
    default:
        raise_error("Bad argument 2 to get_protection()\n");
        
        break;
    }
    
    return ({string}) SECURITY -> query_protection(file, write_priv);
}

/*
 * Returns the current call allows actions which are restricted to
 * the given privilege. The "current call" means all objects in the
 * caller stack down to the last that called unguarded().
 */
int check_privilege(mixed priv) {
    return ({int}) SECURITY -> check_privilege(priv, 1);
}


/*
 * Retrieves if the flag is_wizard is set for the given object
 * this flag should be set in player.c
 */
int query_is_wizard(object player) {
    return 0; //set_is_wizard(player, -1);
}

int query_is_creator(object player) {
    return 0; //set_is_wizard(player, -1);
}

/*
 * Returns true, if <who> is a wizard. The range can be widend
 * by giving a additional <mode> like IW_TESTPLAYER
 */
varargs status is_wizard(mixed who, int mode) {
    mixed temp;
    
    switch (mode) {
    case 0:
    case IC_CREATOR:
        if (objectp(who)) {            
            
            temp = object_info(who, OI_ONCE_INTERACTIVE) && ({int}) who -> query_privilege();            
            if (intp(temp)) {
                return ({int}) temp;
            }
        }
        else if (stringp(who)) {
            temp = who;
        }
        else {
            return ({int}) 0;
        }
        
        return ({status}) SECURITY -> query_is_wizard(temp) && 1;
    case IC_TESTPLAYER:
    case IC_TOOLUSER:
        if (objectp(who)) {
            return (object_info(who, OI_ONCE_INTERACTIVE) &&
                (is_wizard(who, IC_CREATOR)));
        }
        
        if (!stringp(who)) {
            return ({int}) 0;
        }
        
        return (({status}) SECURITY -> query_is_wizard(who));

    default:
        raise_error("Bad argument 2 (mode) to is_wizard()\n");
    }
}

varargs status is_creator(mixed who, int mode) {
    mixed temp;
    
    switch (mode) {
    case 0:
    case IC_CREATOR:
        if (objectp(who)) {            
            
            temp = ({int}) (object_info(who, OI_ONCE_INTERACTIVE) && who -> query_privilege());            
            if (intp(temp)) {
                return ({int}) temp;
            }
        }
        else if (stringp(who)) {
            temp = who;
        }
        else {
            return ({int}) 0;
        }
        
        return ({status}) SECURITY -> query_is_wizard(temp) && 1;
    case IC_TESTPLAYER:
    case IC_TOOLUSER:
        if (objectp(who)) {
            return (object_info(who, OI_ONCE_INTERACTIVE) &&
                (is_wizard(who, IC_CREATOR)));
        }
        
        if (!stringp(who)) {
            return ({int}) 0;
        }
        
        return (({status}) SECURITY -> query_is_wizard(who));

    default:
        raise_error("Bad argument 2 (mode) to is_wizard()\n");
    }
}
