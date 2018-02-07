/*
 * Prepare socket and get twitch user information from a file
 * TODO: SSL sockets
 */
#include "twitch_connect.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#ifdef CREATE_LOGS
#include <time.h>
#endif

pthread_t send_thread;
pthread_t recv_thread;

void init_sr_threads(Twitch *twitch)
{
    pthread_create(&send_thread, NULL, twitch_send_thread, (void *)twitch);
    pthread_create(&recv_thread, NULL, twitch_recv_thread, (void *)twitch);
}

void destroy_sr_threads()
{
    pthread_cancel(send_thread);
    pthread_cancel(recv_thread);
}

/*
 * Threaded socket recv
 * Writes/signals the read queue
 */
void *twitch_recv_thread(void *arg)
{
    char buffer[512 * 2];
    char *message;
    size_t  peeked;
    ssize_t onemsg;
    Twitch *tt = (Twitch *)arg;

    #ifdef CREATE_LOGS
    char log_buffer[64] = { 0 };
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(log_buffer, 13, "_%T.in", timeinfo);
    FILE *log_file;
    log_file = fopen(log_buffer, "w");
    FILE *detail_file;
    strftime(log_buffer, 14, "_%T.txt", timeinfo);
    detail_file = fopen(log_buffer, "w");
    #endif

    //printf("Thread %x starting\n", pthread_self());
    do {
        peeked = recv(tt->sock, buffer, 512 * 2, MSG_PEEK);

        onemsg = 0;
        for (size_t i = 0; i < peeked - 1; i++) {
            if (buffer[i] == '\r' && buffer[i + 1] == '\n') {
                onemsg = i + 2;
                break;
            }
        }

        if (onemsg > 0) {
            message = malloc(onemsg + 1);
            memcpy(message, buffer, onemsg);
            message[onemsg] = '\0';
            #ifdef CREATE_LOGS
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            fputs(message, log_file);
            strftime(log_buffer, 9, "%T", timeinfo);
            fprintf(detail_file, "%s\t%s", log_buffer, message);
            fflush(log_file);
            fflush(detail_file);
            #endif
            // Enqueue into recv_q
            enqueue_line(tt->recv_q, message);

            // Take message out of the buffer
            recv(tt->sock, buffer, onemsg, 0);
        }
    } while (1);
}

/*
 * Threaded socket send
 * Reads/waits on the send queue
 */
void *twitch_send_thread(void *arg)
{
    size_t total;
    size_t len;
    ssize_t sent;
    char *message;
    Twitch *tt = (Twitch *)arg;

    //printf("Thread %x starting\n", pthread_self());
    do {
        dequeue_line(tt->send_q, &message);
        len = strlen(message);

        total = 0;
        do {
            sent = send(tt->sock, message, len - total, 0);

            if (sent == -1)
                perror("Could not send data");
            else
                total += sent;
        } while (sent < total);
        free(message);
    } while (1);
}

/*
 * Pull single line of information out of a file
 */
char *get_info(const char *file)
{
    char *info = malloc(513);
    FILE *fd;

    fd = fopen(file, "r");
    if (fd == NULL) {
        perror("Could not open file");
        free(info);
        return NULL;
    }

    if (fscanf(fd, "%s", info) == EOF) {
        perror("Count not read info");
        free(info);
        info = NULL;
    }

    fclose(fd);

    return info;
}

/*
 * Get the oauth, max length 255
 */
char *get_twitch_oauth(const char *oauthfile)
{
    char *oauth = get_info(oauthfile);

    if (oauth == NULL || OAUTH_LEN < strlen(oauth))
        return NULL;

    return oauth;
}

/*
 * Get the username, max length 26
 */
char *get_twitch_user(const char *userfile)
{
    char *user = get_info(userfile);

    if (user == NULL || USER_LEN < strlen(user))
        return NULL;

    return user;
}

/*
 * Prints host name and IPv4 address
 */
// void print_server(struct sockaddr_in *sock)
// {
//     char *host = malloc(URL_STR_LEN);
//     char *ip   = malloc(INET_ADDRSTRLEN);

//     getnameinfo((struct sockaddr *)sock, sizeof(*sock), host, URL_STR_LEN, NULL, 0, 0);
//     printf("Host: %s\n", host);
//     inet_ntop(AF_INET, (void *)&sock->sin_addr, ip, INET_ADDRSTRLEN);
//     printf("IPv4: %s\n", ip);

//     free(ip);
//     free(host);
// }

/*
 * Connects to irc.chat.twitch.tv at port 6667
 * On success returns a connected socket file descriptor
 * Failure return 0
 */
int get_twitch_socket()
{
    int ret_error;
    int sockfd = 0;
    struct addrinfo *alist;
    struct addrinfo hint = {
        .ai_flags     = AI_V4MAPPED | AI_ADDRCONFIG,
        .ai_family    = AF_INET,
        .ai_socktype  = SOCK_STREAM,
        .ai_protocol  = IPPROTO_TCP,
        .ai_addrlen   = 0,
        .ai_addr      = NULL,
        .ai_canonname = NULL,
        .ai_next      = NULL
    };

    // Get the servers IPv4 address
    ret_error = getaddrinfo("irc.chat.twitch.tv", "6667", &hint, &alist);
    if (ret_error == 0) {
        //print_server((struct sockaddr_in *)alist->ai_addr);

        // Get socket to server
        sockfd = socket(alist->ai_family, alist->ai_socktype, alist->ai_protocol);
        if (sockfd >= 0) {
            // Connect to server
            ret_error = connect(sockfd, alist->ai_addr, alist->ai_addrlen);
            if (ret_error == -1) {
                //perror("Failed to connect");
                close(sockfd);
                sockfd = 0;
            }
        } else { // socket error
            //perror("Failed to init socket");
            sockfd = 0;
        }
    } else { // address error
        //fprintf(stderr, "%s\n", gai_strerror(ret_error));
    }

    freeaddrinfo(alist);

    return sockfd;
}

/*
 * Close the socket
 */
void close_twitch_socket(int twitch_socket)
{
    if (close(twitch_socket) == -1) {
        perror("Failed to close socket");
    } else {
        printf("Socket closed\n");
    }
}
