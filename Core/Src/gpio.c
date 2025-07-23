/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   This file provides code for the configuration
 *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "tim.h"
#include "flow.h"
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
 */
void MX_GPIO_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOG, Toggle_Pin | Error_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : Button_Pin */
    GPIO_InitStruct.Pin = Button_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(Button_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : Toggle_Pin */
    GPIO_InitStruct.Pin = Toggle_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(Toggle_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : Error_Pin */
    GPIO_InitStruct.Pin = Error_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(Error_GPIO_Port, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */
void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    // If we have the pushbutton toggle option selected,
    // the pin is the right one and we haven't started
    // debouncing a previour press, we can start debouncing
    // the current press.
    if ((get_state() == PUSHBUTTON_TOGGLE) &&
        (pin == Button_Pin) &&
        (HAL_TIM_Base_GetState(&htim5) != HAL_TIM_STATE_BUSY))
    {
        if (HAL_TIM_Base_Start_IT(&htim5) != HAL_OK)
        {
            Error_Handler();
        }
    }
}
/* USER CODE END 2 */
