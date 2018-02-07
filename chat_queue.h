#ifndef __chat_queue_h
#define __chat_queue_h

#include <pthread.h>

#define CHAT_MAX_LEN 512
#define MESSAGE_TOTAL 128

typedef struct chat_q {
    int start;
    int end;
    int length;
    char **messages;
    pthread_mutex_t lock;
    pthread_cond_t notempty;
} ChatQueue;

void enqueue_line(ChatQueue *queue, char *message);
void dequeue_line(ChatQueue *queue, char **message);
ChatQueue *init_queue();
void delete_queue(ChatQueue *queue);

#endif /* __chat_queue_h */
