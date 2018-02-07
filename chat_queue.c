#include "chat_queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define INC_POS(x) x = (x + 1) % MESSAGE_TOTAL

void enqueue_line(ChatQueue *queue, char *message)
{
    pthread_mutex_lock(&queue->lock);

    if (queue->length == MESSAGE_TOTAL) {
         fprintf(stderr, "queue_q full, losing message:\n%s", queue->messages[queue->start]);
         dequeue_line(queue, NULL);
    }

    if (queue->length == 0)
        pthread_cond_signal(&queue->notempty);

    queue->messages[queue->end] = message;
    INC_POS(queue->end);
    queue->length++;

    pthread_mutex_unlock(&queue->lock);
}

void dequeue_line(ChatQueue *queue, char **message)
{
    pthread_mutex_lock(&queue->lock);

    if (queue->length == 0)
        pthread_cond_wait(&queue->notempty, &queue->lock);

    if (message != NULL)
        *message = queue->messages[queue->start];
    else
        free(queue->messages[queue->start]);

    INC_POS(queue->start);
    queue->length--;

    pthread_mutex_unlock(&queue->lock);
}

ChatQueue *init_queue()
{
    ChatQueue *queue = malloc(sizeof(ChatQueue));
    queue->messages  = malloc(sizeof(char *) * MESSAGE_TOTAL);
    queue->start     = 0;
    queue->end       = 0;
    queue->length    = 0;
    memset(queue->messages, 0, sizeof(char *) * MESSAGE_TOTAL);
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutex_init(&queue->lock, &mattr);
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_cond_init(&queue->notempty, &cattr);
    return queue;
}

void delete_queue(ChatQueue *queue)
{
    while (queue->length > 0) {
        free(queue->messages[queue->start % MESSAGE_TOTAL]);
        queue->start++;
        queue->length--;
    }
    free(queue->messages);
    free(queue);
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->notempty);
}

