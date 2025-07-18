/*
 * queue.h
 *
 *  Created on: Jul 18, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_QUEUE_H_
#define INC_QUEUE_H_

#include <stdint.h>
#include <stdbool.h>

#define QUEUE_SIZE 128
// Circular queue
typedef struct {
	uint8_t queue[QUEUE_SIZE];
	int top;
	int bottom;
} queue;

void queue_init(queue *q);
bool queue_push(queue *q, uint8_t elem);
bool queue_pop(queue *q, uint8_t *elem);

#endif /* INC_QUEUE_H_ */
