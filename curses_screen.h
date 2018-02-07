#ifndef __curses_screen_h
#define __curses_screen_h

#include "twitch.h"
#include "message_parse.h"

void init_win_threads();
void destroy_win_threads();
int fmt_message(char *buffer, TwitchMsg *message);
void *recv_messages(void *arg);
void *send_messages(void *arg);
void init_screens(Twitch *twitch_connection);

#endif /* __curses_screen_h */