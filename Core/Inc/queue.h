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
typedef struct
{
    uint32_t queue[QUEUE_SIZE];
    uint32_t top;
    uint32_t bottom;
} queue_t;

void queue_init(queue_t *q);
void queue_push_overwrite(queue_t *q, uint32_t elem);
bool queue_pop(queue_t *q, uint32_t *elem);
void queue_clear(queue_t *q);
bool queue_peek(queue_t *q, uint32_t *elem);
bool queue_empty(queue_t *q);

#endif /* INC_QUEUE_H_ */
