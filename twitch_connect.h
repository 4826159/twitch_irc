#ifndef __twitch_connect_h
#define __twitch_connect_h

#include "twitch.h"

void init_sr_threads(Twitch *twitch);
void destroy_sr_threads();
void *twitch_recv_thread(void *arg);
void *twitch_send_thread(void *arg);
char *get_twitch_user(const char *userfile);
char *get_twitch_oauth(const char *oauthfile);
int get_twitch_socket();
void close_twitch_socket(int twitch_socket);

#endif /* __twitch_connect_h */
