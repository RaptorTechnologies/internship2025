/*
 * uart.h
 *
 *  Created on: Jul 18, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "queue.h"

extern queue command_queue;

void start_UART(UART_HandleTypeDef *huart);

#endif /* INC_UART_H_ */
