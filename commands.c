#include "commands.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void send_PASS(Twitch *twitch, const char *oauth)
{
    const char *fmt = "PASS oauth:%s\r\n";
    char *message = malloc(strlen(fmt) + strlen(oauth));

    sprintf(message, fmt, oauth);

    enqueue_line(twitch->send_q, message);
}

void send_NICK(Twitch *twitch, const char *nick)
{
    const char *fmt = "NICK %s\r\n";
    char *message = malloc(strlen(fmt) + strlen(nick));

    sprintf(message, fmt, nick);

    enqueue_line(twitch->send_q, message);
}

void send_JOIN(Twitch *twitch, const char *channel)
{
    const char *fmt = "JOIN #%s\r\n";
    char *message = malloc(strlen(fmt) + strlen(channel));

    sprintf(message, fmt, channel);

    enqueue_line(twitch->send_q, message);
}

void send_PART(Twitch *twitch, const char *channel)
{
    const char *fmt = "PART #%s\r\n";
    char *message = malloc(strlen(fmt) + strlen(channel));

    sprintf(message, fmt, channel);

    enqueue_line(twitch->send_q, message);
}

void send_PRIVMSG(Twitch *twitch, const char *channel, const char *privmsg)
{
    const char *send_fmt = "PRIVMSG #%s :%s\r\n";
    const char *recv_fmt = ":%s!%s@%s.tmi.twitch.tv PRIVMSG #%s :%s\r\n";
    int send_length = strlen(send_fmt) + strlen(channel) + strlen(privmsg);
    char *send_msg = malloc(send_length);
    sprintf(send_msg, send_fmt, channel, privmsg);

    int recv_length = strlen(recv_fmt) + 3 * strlen(twitch->user) + strlen(channel) + strlen(privmsg);
    char *recv_msg = malloc(recv_length);
    sprintf(recv_msg, recv_fmt, twitch->user, twitch->user, twitch->user, channel, privmsg);

    enqueue_line(twitch->send_q, send_msg);
    enqueue_line(twitch->recv_q, recv_msg);
}

void send_PONG(Twitch *twitch)
{
    const char *fmt = "PONG\r\n";
    char *message = malloc(strlen(fmt));

    sprintf(message, fmt);

    enqueue_line(twitch->send_q, message);
}