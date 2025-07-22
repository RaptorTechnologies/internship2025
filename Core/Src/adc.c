/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    adc.c
 * @brief   This file provides code for the configuration
 *          of the ADC instances.
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
#include "adc.h"

/* USER CODE BEGIN 0 */
#include "flow.h"
#include "tim.h"
/* USER CODE END 0 */

ADC_HandleTypeDef hadc3;

/* ADC3 init function */
void MX_ADC3_Init(void)
{

    /* USER CODE BEGIN ADC3_Init 0 */

    /* USER CODE END ADC3_Init 0 */

    ADC_ChannelConfTypeDef sConfig = { 0 };

    /* USER CODE BEGIN ADC3_Init 1 */

    /* USER CODE END ADC3_Init 1 */

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
     */
    hadc3.Instance = ADC3;
    hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc3.Init.Resolution = ADC_RESOLUTION_12B;
    hadc3.Init.ScanConvMode = ENABLE;
    hadc3.Init.ContinuousConvMode = DISABLE;
    hadc3.Init.DiscontinuousConvMode = ENABLE;
    hadc3.Init.NbrOfDiscConversion = 1;
    hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc3.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
    hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc3.Init.NbrOfConversion = 1;
    hadc3.Init.DMAContinuousRequests = DISABLE;
    hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc3) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
     */
    sConfig.Channel = ADC_CHANNEL_8;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN ADC3_Init 2 */

    /* USER CODE END ADC3_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    if (adcHandle->Instance == ADC3)
    {
        /* USER CODE BEGIN ADC3_MspInit 0 */

        /* USER CODE END ADC3_MspInit 0 */
        /* ADC3 clock enable */
        __HAL_RCC_ADC3_CLK_ENABLE();

        __HAL_RCC_GPIOF_CLK_ENABLE();
        /**ADC3 GPIO Configuration
         PF10     ------> ADC3_IN8
         */
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

        /* ADC3 interrupt Init */
        HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(ADC_IRQn);
        /* USER CODE BEGIN ADC3_MspInit 1 */

        /* USER CODE END ADC3_MspInit 1 */
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *adcHandle)
{

    if (adcHandle->Instance == ADC3)
    {
        /* USER CODE BEGIN ADC3_MspDeInit 0 */

        /* USER CODE END ADC3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_ADC3_CLK_DISABLE();

        /**ADC3 GPIO Configuration
         PF10     ------> ADC3_IN8
         */
        HAL_GPIO_DeInit(GPIOF, GPIO_PIN_10);

        /* ADC3 interrupt Deinit */
        HAL_NVIC_DisableIRQ(ADC_IRQn);
        /* USER CODE BEGIN ADC3_MspDeInit 1 */

        /* USER CODE END ADC3_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */
const uint32_t TIMER_MAX = 1000;
const uint32_t TIMER_MIN = 100;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC3)
    {
        uint32_t potentiometer_value = HAL_ADC_GetValue(hadc);
        if (get_state() == ADC_LED_TOGGLE)
        {
            // Set the timer to a value between TIMER_MIN and TIMER_MAX ms
            // From the ADC we get values between 0 and 0xFFF (12 bit precision)
            // We normalize it by dividing with 0xFFF, then scale it to (TIMER_MAX - TIMER_MIN) and add TIMER_MIN
            htim2.Instance->ARR =
                    (potentiometer_value * (TIMER_MAX - TIMER_MIN)) / 0x0FFF + TIMER_MIN;

            // If the timer is above our newly set value, we restart the count.
            if (htim2.Instance->CNT > htim2.Instance->ARR)
            {
                htim2.Instance->CNT = 0;
            }
        }
        else if (get_state() == ADC_LED_TOGGLE_PWM)
        {
            htim3.Instance->CCR3 = potentiometer_value;
        }
    }
}
/* USER CODE END 1 */
