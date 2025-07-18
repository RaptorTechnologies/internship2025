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
void queue_init(queue *q) {
	memset(q->queue, 0, sizeof(*q));
}

/**
  * @brief  This function tries to push an element in the queue.
  * @retval True if succeeded, false if queue is full.
  */
bool queue_push(queue *q, uint8_t elem) {
	if ((q->top + 1) % QUEUE_SIZE == q->bottom) {
		return false;
	}

	q->queue[q->top] = elem;
	q->top = (q->top + 1) % QUEUE_SIZE;
	return true;
}

/**
  * @brief  This function tries to pop an element from the queue.
  * @retval True if succeeded, false if queue is empty.
  */
bool queue_pop(queue *q, uint8_t *elem) {
	if (q->top == q->bottom) {
		return false;
	}

	*elem = q->queue[q->bottom];
	q->bottom = (q->bottom + 1) % QUEUE_SIZE;
	return true;
}
