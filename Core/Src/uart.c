/*
 * uart.c
 *
 *  Created on: Jul 18, 2025
 *      Author: Andrei Trif
 */

#include <stdlib.h>
#include "main.h"
#include "uart.h"

queue command_queue = { 0 };

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

volatile uint8_t char_rx;
static UART_HandleTypeDef *sel_uart;

void start_UART(UART_HandleTypeDef *huart) {
	sel_uart = huart;
	if (HAL_UART_Receive_IT(sel_uart, (uint8_t*) &char_rx, 1) != HAL_OK) {
		Error_Handler();
	}
}

PUTCHAR_PROTOTYPE {
	HAL_UART_Transmit(sel_uart, (uint8_t*) &ch, 1, 0xFFFF);

	return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == UART4) {
		queue_push(&command_queue, char_rx);

		if (HAL_UART_Receive_IT(sel_uart, (uint8_t*) &char_rx, 1) != HAL_OK) {
			Error_Handler();
		}
	}
}
