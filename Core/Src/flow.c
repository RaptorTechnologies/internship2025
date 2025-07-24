/*
 * flow.c
 *
 *  Created on: Jul 21, 2025
 *      Author: Andrei Trif
 */

#include "flow.h"
#include "tim.h"
#include "adc.h"

#define HAL_CHECK(expr) \
    if ((expr) != HAL_OK) { \
        Error_Handler(); \
    }

static state_t state = WAITING_OPTION;

static uint32_t task_options[2] = {
    [BUTTON_INTERVAL_RECORDING_TIME] = 10000,
    [BUTTON_INTERVAL_KEEP_LED_ON_TIME] = 1000
};

void set_option(option_t opt, uint32_t value)
{
    task_options[opt] = value;
}

uint32_t get_option(option_t opt)
{
    return task_options[opt];
}

/**
 * @brief Set the new option to run and initialize it if necessary. If the current state isn't WAITING_OPTION, it stops the current option.
 * @retval None
 */
void init_option(state_t s)
{
    state = s;

    // Start option.
    switch (state)
    {
        case WAITING_OPTION:
            break;
        case PUSHBUTTON_TOGGLE:
            break;
        case TIMER_TOGGLE:
        {
            HAL_CHECK(HAL_TIM_Base_Start_IT(&htim4));
            break;
        }
        case ADC_READING:
        {
            HAL_CHECK(HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1));
            HAL_CHECK(HAL_ADC_Start_IT(&hadc3));
            break;
        }
        case ADC_LED_TOGGLE:
        {
            HAL_CHECK(HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1));
            HAL_CHECK(HAL_TIM_Base_Start_IT(&htim2));
            HAL_CHECK(HAL_ADC_Start_IT(&hadc3));
            break;
        }
        case ADC_LED_TOGGLE_PWM:
        {
            HAL_CHECK(HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1));
            HAL_CHECK(HAL_ADC_Start_IT(&hadc3));
            HAL_CHECK(HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3));
            break;
        }
        case BUTTON_INTERVAL:
        {
            htim9.Instance->ARR = get_option(BUTTON_INTERVAL_RECORDING_TIME);
            HAL_CHECK(HAL_TIM_Base_Start_IT(&htim9));
            break;
        }
        default:
            break;
    }
}

/**
 * @brief Set the state to waiting and deinitialize the previous option if necessary.
 * @retval None
 */
#include "stdio.h"
void deinit_options(void)
{
    // Stop option.
    switch (state)
    {
        case WAITING_OPTION:
            return;
        case PUSHBUTTON_TOGGLE:
            break;
        case TIMER_TOGGLE:
        {
            HAL_CHECK(HAL_TIM_Base_Stop_IT(&htim4));
            break;
        }
        case ADC_READING:
        {
            HAL_CHECK(HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1));
            HAL_CHECK(HAL_ADC_Stop_IT(&hadc3));
            break;
        }
        case ADC_LED_TOGGLE:
        {
            HAL_CHECK(HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1));
            HAL_CHECK(HAL_TIM_Base_Stop_IT(&htim2));
            HAL_CHECK(HAL_ADC_Stop_IT(&hadc3));
            break;
        }
        case ADC_LED_TOGGLE_PWM:
        {
            HAL_CHECK(HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1));
            HAL_CHECK(HAL_ADC_Stop_IT(&hadc3));
            HAL_CHECK(HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3));
            break;
        }
        case BUTTON_INTERVAL:
        {
            reset_recordings();
            HAL_CHECK(HAL_TIM_Base_Stop_IT(&htim9));

            // We might be in the recording phase so TIM6 might not have started yet.
            HAL_TIM_Base_Stop_IT(&htim6);
            break;
        }
        default:
            break;
    }

    state = WAITING_OPTION;
}

/**
 * @brief Set the state.
 * @retval None
 */
void set_state(state_t s)
{
    if (s != state)
    {
        deinit_options();
        init_option(s);
    }
}

/**
 * @brief Get the current state.
 * @retval The state
 */
state_t get_state(void)
{
    return state;
}
