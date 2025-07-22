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
 * @brief Set the new option to run and initialize it if necessary. If WAITING_OPTION is provided or if the current state isn't WAITING_OPTION, the function will not change anything.
 * @retval None
 */
void start_option(state_t s)
{
    // We must be in WAITING_OPTION and we must go towards any other state.
    if (s == WAITING_OPTION || state != WAITING_OPTION)
    {
        return;
    }

    state = s;

    // Start option.
    switch (state)
    {
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
 * @brief Get the current state.
 * @retval The state
 */
state_t get_state(void)
{
    return state;
}

/**
 * @brief Set the state to waiting and deinitialize the previous option if necessary.
 * @retval None
 */
void stop_options(void)
{
    // Stop option.
    switch (state)
    {
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
