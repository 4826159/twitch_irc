#include "curses_screen.h"
#include "commands.h"

#include <ctype.h>
#include <curses.h>
#include <locale.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

pthread_t recv_win_thread;
pthread_t send_win_thread;
WINDOW *recv_win;
WINDOW *send_win;
pthread_mutex_t curs_lock;
Twitch *twitch;

#define MAX_CHAT 100
#define MIN(x, y) (x < y ? x : y)

void init_win_threads()
{
    pthread_create(&recv_win_thread, NULL, recv_messages, NULL);
    pthread_create(&send_win_thread, NULL, send_messages, NULL);
}

void destroy_win_threads()
{
    pthread_cancel(recv_win_thread);
    pthread_cancel(send_win_thread);
}

int fmt_message(char *buffer, TwitchMsg *message)
{
    int length;
    int c_len = strlen(message->command->name);
    

    if (strncmp("PRIVMSG", message->command->name, MIN(c_len, 7)) == 0) {
        
        char *nick = message->prefix->nick;
        char *trail = message->params->trailing;
        length = snprintf(buffer, MSG_MAX, "%s: %s", nick, trail);
    }
    else if (strncmp("PING", message->command->name, MIN(c_len, 4)) == 0) {
        length = snprintf(buffer, MSG_MAX, "PING recv'd from %s, sent PONG", message->params->trailing);
    }
    else {
        c_len = strlen(message->message) - 1;
        length = snprintf(buffer, c_len, message->message);
    }

    return length;
}

void *recv_messages(void *arg)
{
    TwitchMsg *chat[MAX_CHAT] = { 0 };
    int chat_start = 0;
    int chat_count = 0;
    char buffer[MSG_MAX];
    char *line = NULL;
    int recv_rows, recv_cols;

    while (1) {
        dequeue_line(twitch->recv_q, &line);

        if (chat_count >= MAX_CHAT) {
            free_message(chat[chat_start]);
            chat_start = (chat_start + 1) % MAX_CHAT;
            chat_count--;
        }

        TwitchMsg *new_msg = parse_message(line);
        if (strncmp("PING", new_msg->command->name, MIN(4, strlen(new_msg->command->name))))
            send_PONG(twitch);
        chat[(chat_start + chat_count) % MAX_CHAT] = new_msg;
        free(line);
        chat_count++;

        pthread_mutex_lock(&curs_lock);

        getmaxyx(recv_win, recv_rows, recv_cols);
        wclear(recv_win);
        
        for (int i = 1, y = recv_rows; i <= chat_count; i++) {
            TwitchMsg *msg = chat[(chat_start + chat_count - i) % MAX_CHAT];
            int line_len;
            int line_total;
            line_len = fmt_message(buffer, msg);

            line_total = (line_len - 1) / recv_cols + 1;

            y -= line_total;

            if (y < 0)
                break;

            mvwprintw(recv_win, y, 0, buffer);
        }
        
        wrefresh(recv_win);
        pthread_mutex_unlock(&curs_lock);
    }
}

void *send_messages(void *arg)
{
    int ch;
    char buffer[MSG_MAX] = { 0 };
    int length = 0;
    while (1) {
        pthread_mutex_lock(&curs_lock);

        wclear(send_win);
        mvwhline(send_win, 0, 0, '-', COLS);
        mvwprintw(send_win, 1, 0, buffer);
        wrefresh(send_win);

        pthread_mutex_unlock(&curs_lock);

        ch = wgetch(send_win);

        if (isprint(ch) && length < MSG_MAX - 1) {
            buffer[length] = (char)ch;
            length++;
            buffer[length] = 0;
        }
        else if (ch == '\x7F' && length > 0) {
            length--;
            buffer[length] = 0;
        }
        else if (ch == '\n' && length > 0) {
            send_PRIVMSG(twitch, CHANNEL, buffer);
            //length = 0;
            //buffer[length] = 0;
        }
    }
}

void init_screens(Twitch *twitch_connection)
{
    twitch = twitch_connection;

    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();

    recv_win = newwin(LINES - 4, COLS, 0, 0);
    send_win = newwin(4, COLS, LINES - 4, 0);

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutex_init(&curs_lock, &mattr);
}