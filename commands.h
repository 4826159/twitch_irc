#include "twitch.h"

void send_PASS(Twitch *twitch, const char *oauth);
void send_NICK(Twitch *twitch, const char *nick);
void send_JOIN(Twitch *twitch, const char *channel);
void send_PART(Twitch *twitch, const char *channel);
void send_PRIVMSG(Twitch *twitch, const char *channel, const char *privmsg);
void send_PONG(Twitch *twitch);