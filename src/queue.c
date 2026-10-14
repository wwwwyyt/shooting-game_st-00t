#include "queue.h"

int QUE_Create(Queue *queue, int size)
{
        if (!queue || !size)
        {
                return 1;
        }

        SDL_memset(queue, 0, sizeof(Queue));
        queue->q = SDL_malloc(size * sizeof(void *));
        if (queue->q)
                SDL_memset(queue->q, 0, size * sizeof(void *));
        queue->size = size;
        return 0;
}

void QUE_Destroy(Queue *queue)
{
        SDL_free(queue->q);
}

int QUE_Push(Queue *queue, void *item)
{
        if (!queue)
        {
                return 1;
        }
        if (queue->head == queue->tail && queue->cnt == queue->size)
        {
                return 1;
        }
        queue->q[queue->head] = item;
        queue->cnt += 1;
        queue->head = (queue->head + 1) % queue->size;
        return 0;
}

void *QUE_Pop(Queue *queue)
{
        if (!queue)
        {
                SDL_SetError("Pointer to queue is null.");
                return NULL;
        }
        if (queue->head == queue->tail && queue->cnt == 0)
        {
                return NULL;
        }
        void *item = queue->q[queue->tail];
        queue->q[queue->tail] = NULL;
        queue->cnt -= 1;
        queue->tail = (queue->tail + 1) % queue->size;
        return item;
}

void *QUE_Head(Queue *queue)
{
        return queue->q[queue->head];
}

void *QUE_Tail(Queue *queue)
{
        return queue->q[queue->tail];
}

int QUE_IsFull(Queue *queue)
{
        return queue->cnt >= queue->size;
}
