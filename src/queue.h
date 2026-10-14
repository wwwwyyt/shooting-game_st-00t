#ifndef SG_QUEUE_H
#define SG_QUEUE_H

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_error.h>

typedef struct
{
        void **q;
        int size;
        int cnt;
        int head, tail;
} Queue;

/** @brief 创建循环队列
 *
 *  @return 成功返回 0 ，失败返回 1
 */
int QUE_Create(Queue *queue, int size);

/** @brief 销毁循环队列 */
void QUE_Destroy(Queue *queue);

/** @brief 向队列中压入元素 */
int QUE_Push(Queue *queue, void *item);

/** @brief 从队列末尾弹出元素 */
void *QUE_Pop(Queue *queue);

/** @brief 返回指向队首元素 */
void *QUE_Head(Queue *queue);

/** @brief 返回指向队尾元素 */
void *QUE_Tail(Queue *queue);

/** @brief 判断一个队列是否已满 */
int QUE_IsFull(Queue *queue);

#endif /* SG_QUEUE_H */
