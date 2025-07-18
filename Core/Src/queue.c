/*
 * queue.c
 *
 *  Created on: Jul 18, 2025
 *      Author: Andrei Trif
 */

#include "queue.h"
#include <string.h>

/**
 * @brief  Initialize the queue pointed to by q.
 * @retval None
 */
void queue_init(queue_t *q) {
	memset(q, 0, sizeof(*q));
}

/**
 * @brief  This function pushes elem into the queue. If queue is full, it overwrites the oldest element.
 * @retval None
 */
void queue_push_overwrite(queue_t *q, uint8_t elem) {
	if (((q->top + 1) % QUEUE_SIZE) == q->bottom) {
		q->bottom = (q->bottom + 1) % QUEUE_SIZE;
	}

	q->queue[q->top] = elem;
	q->top = (q->top + 1) % QUEUE_SIZE;
}

/**
 * @brief  This function tries to pop an element from the queue.
 * @retval True if succeeded, false if queue is empty.
 */
bool queue_pop(queue_t *q, uint8_t *elem) {
	if (q->top == q->bottom) {
		return false;
	}

	*elem = q->queue[q->bottom];
	q->bottom = (q->bottom + 1) % QUEUE_SIZE;
	return true;
}
