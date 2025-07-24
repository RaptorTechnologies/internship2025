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

static uint32_t state = 0;

typedef enum
{
    RES_TIM4, RES_TIM1, RES_ADC3, RES_TIM2, RES_TIM3
} resource_t;

static uint8_t resources[5] = { 0 };
bool is_resource_on(resource_t r)
{
    return resources[r] != 0;
}

void start_resource(resource_t r)
{
    if (!is_resource_on(r))
    {
        switch (r)
        {
            case RES_TIM4:
            {
                HAL_CHECK(HAL_TIM_Base_Start_IT(&htim4));
            }
                break;
            case RES_TIM1:
            {
                HAL_CHECK(HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1));
            }
                break;
            case RES_ADC3:
            {
                HAL_CHECK(HAL_ADC_Start_IT(&hadc3));
            }
                break;
            case RES_TIM2:
            {
                HAL_CHECK(HAL_TIM_Base_Start_IT(&htim2));
            }
                break;
            case RES_TIM3:
            {
                HAL_CHECK(HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3));
            }
                break;
        }
    }
    ++resources[r];
}

void stop_resource(resource_t r)
{
    --resources[r];
    if (!is_resource_on(r))
    {
        switch (r)
        {
            case RES_TIM4:
            {
                HAL_CHECK(HAL_TIM_Base_Stop_IT(&htim4));
            }
                break;
            case RES_TIM1:
            {
                HAL_CHECK(HAL_TIM_OC_Stop_IT(&htim1, TIM_CHANNEL_1));
            }
                break;
            case RES_ADC3:
            {
                HAL_CHECK(HAL_ADC_Stop_IT(&hadc3));
            }
                break;
            case RES_TIM2:
            {
                HAL_CHECK(HAL_TIM_Base_Stop_IT(&htim2));
            }
                break;
            case RES_TIM3:
            {
                HAL_CHECK(HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3));
            }
                break;
        }
    }
}

/**
 * @brief Set the new option to run and initialize it if necessary. If the current state isn't WAITING_OPTION, it stops the current option.
 * @retval None
 */
void init_option(state_t s)
{
    state |= s;

    // Start option.
    switch (s)
    {
        case WAITING_OPTION:
            break;
        case PUSHBUTTON_TOGGLE:
            break;
        case TIMER_TOGGLE:
        {
            start_resource(RES_TIM4);
        }
            break;
        case ADC_READING:
        {
            start_resource(RES_TIM1);
            start_resource(RES_ADC3);
        }
            break;
        case ADC_LED_TOGGLE:
        {
            start_resource(RES_TIM1);
            start_resource(RES_ADC3);
            start_resource(RES_TIM2);
        }
            break;
        case ADC_LED_TOGGLE_PWM:
        {
            start_resource(RES_TIM1);
            start_resource(RES_ADC3);
            start_resource(RES_TIM3);
        }
            break;
        default:
            break;
    }
}

/**
 * @brief Set the state to waiting and deinitialize the previous option if necessary.
 * @retval None
 */
void deinit_option(state_t s)
{
    // Stop option.
    switch (s)
    {
        case WAITING_OPTION:
            break;
        case PUSHBUTTON_TOGGLE:
            break;
        case TIMER_TOGGLE:
        {
            stop_resource(RES_TIM4);
        }
            break;
        case ADC_READING:
        {
            stop_resource(RES_TIM1);
            stop_resource(RES_ADC3);
        }
            break;
        case ADC_LED_TOGGLE:
        {
            stop_resource(RES_TIM1);
            stop_resource(RES_ADC3);
            stop_resource(RES_TIM2);
        }
            break;
        case ADC_LED_TOGGLE_PWM:
        {
            stop_resource(RES_TIM1);
            stop_resource(RES_ADC3);
            stop_resource(RES_TIM3);
        }
            break;
        default:
            break;
    }

    state &= ~s;
}

/**
 * @brief Set the state.
 * @retval None
 */
void toggle_state(state_t s)
{
    if (state & s)
    {
        deinit_option(s);
    }
    else
    {
        init_option(s);
    }
}

/**
 * @brief Get the current state.
 * @retval The state
 */
bool is_state_on(state_t s)
{
    return (state & s) != 0;
}
