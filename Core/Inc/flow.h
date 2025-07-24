/*
 * flow.h
 *
 *  Created on: Jul 21, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_FLOW_H_
#define INC_FLOW_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    WAITING_OPTION = 0,
    PUSHBUTTON_TOGGLE = 1,
    TIMER_TOGGLE = 2,
    ADC_READING = 4,
    ADC_LED_TOGGLE = 8,
    ADC_LED_TOGGLE_PWM = 16,
    BUTTON_INTERVAL = 32
} state_t;

void toggle_state(state_t s);
bool is_state_on(state_t s);

typedef enum
{
    BUTTON_INTERVAL_RECORDING_TIME, BUTTON_INTERVAL_KEEP_LED_ON_TIME,
} option_t;
void set_option(option_t opt, uint32_t value);
uint32_t get_option(option_t opt);

void deinit_options(void);
void init_option(state_t s);

#endif /* INC_FLOW_H_ */
