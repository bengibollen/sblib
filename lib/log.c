#pragma strict_types
#pragma save_types

#include "/inc/log.h"
#include "/sys/debug_message.h"

private nosave int current_log_level = LOG_LEVEL_TRACE;  // Default level

public varargs void log(int level, string msg, varargs mixed args) {
    if (level > current_log_level)
        return;

    string prefix;
    switch(level) {
        case LOG_LEVEL_ERROR: prefix = LOG_PREFIX_ERROR; break;
        case LOG_LEVEL_WARN:  prefix = LOG_PREFIX_WARN;  break;
        case LOG_LEVEL_INFO:  prefix = LOG_PREFIX_INFO;  break;
        case LOG_LEVEL_DEBUG: prefix = LOG_PREFIX_DEBUG; break;
        case LOG_LEVEL_TRACE: prefix = LOG_PREFIX_TRACE; break;
        default:             prefix = "?????";          break;
    }
    
    if (sizeof(args))
        msg = sprintf(msg, args...);

    debug_message(sprintf("[%s] %s\n", prefix, msg), DMSG_STDOUT | DMSG_STDERR | DMSG_LOGFILE | DMSG_STAMP);
}

public void error(string msg, varargs mixed args) {
    log(LOG_LEVEL_ERROR, msg, args...);
}

public void warn(string msg, varargs mixed args) {
    log(LOG_LEVEL_WARN, msg, args...);
}

public void info(string msg, varargs mixed args) {
    log(LOG_LEVEL_INFO, msg, args...);
}

public void debug(string msg, varargs mixed args) {
    log(LOG_LEVEL_DEBUG, msg, args...);
}

public void trace(string msg, varargs mixed args) {
    log(LOG_LEVEL_TRACE, msg, args...);
}

public void set_log_level(int level) {
    if (level >= LOG_LEVEL_ERROR && level <= LOG_LEVEL_TRACE)
        current_log_level = level;
}

public int query_log_level() {
    return current_log_level;
}
