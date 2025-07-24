/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    usart.c
 * @brief   This file provides code for the configuration
 *          of the USART instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
queue_t command_queue = { 0 };

volatile uint8_t char_rx;

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* USER CODE END 0 */

UART_HandleTypeDef huart4;

/* UART4 init function */
void MX_UART4_Init(void)
{

    /* USER CODE BEGIN UART4_Init 0 */

    /* USER CODE END UART4_Init 0 */

    /* USER CODE BEGIN UART4_Init 1 */

    /* USER CODE END UART4_Init 1 */
    huart4.Instance = UART4;
    huart4.Init.BaudRate = BAUD_RATE;
    huart4.Init.WordLength = UART_WORDLENGTH_9B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_EVEN;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart4) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN UART4_Init 2 */
    if (HAL_UART_Receive_IT(&huart4, (uint8_t*) &char_rx, 1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE END UART4_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    if (uartHandle->Instance == UART4)
    {
        /* USER CODE BEGIN UART4_MspInit 0 */

        /* USER CODE END UART4_MspInit 0 */
        /* UART4 clock enable */
        __HAL_RCC_UART4_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**UART4 GPIO Configuration
         PC11     ------> UART4_RX
         PC10     ------> UART4_TX
         */
        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /* UART4 interrupt Init */
        HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(UART4_IRQn);
        /* USER CODE BEGIN UART4_MspInit 1 */

        /* USER CODE END UART4_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{

    if (uartHandle->Instance == UART4)
    {
        /* USER CODE BEGIN UART4_MspDeInit 0 */

        /* USER CODE END UART4_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_UART4_CLK_DISABLE();

        /**UART4 GPIO Configuration
         PC11     ------> UART4_RX
         PC10     ------> UART4_TX
         */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_11 | GPIO_PIN_10);

        /* UART4 interrupt Deinit */
        HAL_NVIC_DisableIRQ(UART4_IRQn);
        /* USER CODE BEGIN UART4_MspDeInit 1 */

        /* USER CODE END UART4_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

// printf uses this function to write its output character by character
bool command_queue_pop(uint8_t *s)
{
    return queue_pop(&command_queue, s);
}

PUTCHAR_PROTOTYPE
{
    // Transmit the character to UART4 in blocking mode with the max timeout.
    HAL_UART_Transmit(&huart4, (uint8_t*) &ch, 1, 0xFFFF);

    return ch;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        queue_push_overwrite(&command_queue, char_rx);

        if (HAL_UART_Receive_IT(huart, (uint8_t*) &char_rx, 1) != HAL_OK)
        {
            Error_Handler();
        }
    }
}
/* USER CODE END 1 */
