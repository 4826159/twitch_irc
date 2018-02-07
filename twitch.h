#ifndef __twitch_h
#define __twitch_h

#include "chat_queue.h"

#define URL_STR_LEN 2083
#define OAUTH_LEN 255
#define USER_LEN 26
#define CHANNEL "drdisrespectlive"

typedef struct twitch_t {
    struct chat_q *recv_q;
    struct chat_q *send_q;
    char *user;
    char *oauth;
    int sock;
} Twitch;

#endif /* __twitch_h */
