#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>
#include <locale.h>
#include <unistd.h>

//#include <curses.h>

#include "chat_queue.h"
#include "commands.h"
#include "curses_screen.h"
#include "message_parse.h"
#include "twitch_connect.h"
#include "twitch.h"

Twitch *twitch;

void init_twitch_t()
{
    twitch = malloc(sizeof(Twitch));

    twitch->recv_q = init_queue();
    twitch->send_q = init_queue();
    twitch->user   = get_twitch_user("twitch.user");
    twitch->oauth  = get_twitch_oauth("twitch.token");
    twitch->sock   = get_twitch_socket();
}

void destroy_twitch_t()
{
    delete_queue(twitch->recv_q);
    delete_queue(twitch->send_q);
    free(twitch->user);
    free(twitch->oauth);
    close_twitch_socket(twitch->sock);
    free(twitch);
}

void simple_echo(TwitchMsg *msg)
{
    if (msg->prefix != NULL)
        printf("> %s ", msg->prefix->host);
    
    printf("%s ", msg->command->name);

    MsgParam *p = msg->params;
    for (int i = 0; i < p->count; i++) {
        printf("%s ", p->param[i]);
    }
    printf("\n");
}

int main(int argc, const char *argv[])
{
    init_twitch_t();
    init_sr_threads(twitch);

    init_screens(twitch);
    init_win_threads();

    send_PASS(twitch, twitch->oauth);
    send_NICK(twitch, twitch->user);
    send_JOIN(twitch, CHANNEL);

    //recv_messages(NULL);
    while (1)
        sleep(1);
    //curses_start();

    //destroy_win_threads();
    destroy_sr_threads();
    destroy_twitch_t();
}
