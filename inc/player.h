#ifndef _PLAYER_H
#define _PLAYER_H

// Player states
#define PLAYER_STATE_LOADING    0
#define PLAYER_STATE_PLAYING    1
#define PLAYER_STATE_EDITING    2
#define PLAYER_STATE_AFK       3

// Basic commands every player should have
#define PLAYER_COMMANDS ([ \
    "look"      : "cmd_look", \
    "quit"      : "cmd_quit", \
    "help"      : "cmd_help", \
    "say"       : "cmd_say", \
    "who"       : "cmd_who", \
])

#endif
