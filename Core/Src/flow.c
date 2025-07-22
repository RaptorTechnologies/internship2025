/*
 * flow.c
 *
 *  Created on: Jul 21, 2025
 *      Author: Andrei Trif
 */

#include "flow.h"

static state_t state = WAITING_OPTION;

/**
 * @brief Set the new option to run and initialize it if necessary. If WAITING_OPTION is provided, the function will not change anything.
 * @retval None
 */
void start_option(state_t s)
{
    if (s == WAITING_OPTION)
    {
        return;
    }

    state = s;
}

/**
 * @brief Get the current state.
 * @retval None
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
    state = WAITING_OPTION;
}
