/*
 * flow.c
 *
 *  Created on: Jul 21, 2025
 *      Author: Andrei Trif
 */

#include "flow.h"
#include "tim.h"

static state_t state = WAITING_OPTION;

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
            if (HAL_TIM_Base_Start_IT(&htim4) != HAL_OK)
            {
                Error_Handler();
            }
        }
            break;
        case ADC_READING:
            break;
        case ADC_LED_TOGGLE:
            break;
        case ADC_LED_TOGGLE_PWM:
            break;
        default:
            break;
    }
}

/**
 * @brief Set the state to waiting and deinitialize the previous option if necessary.
 * @retval None
 */
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
            if (HAL_TIM_Base_Stop_IT(&htim4) != HAL_OK)
            {
                Error_Handler();
            }
        }
            break;
        case ADC_READING:
            break;
        case ADC_LED_TOGGLE:
            break;
        case ADC_LED_TOGGLE_PWM:
            break;
        default:
            break;
    }

    state = WAITING_OPTION;
}

/**
 * @brief Set the state.
 * @retval None
 */
void set_state(state_t s) {
    if (s != state) {
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

