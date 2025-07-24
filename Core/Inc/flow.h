/*
 * flow.h
 *
 *  Created on: Jul 21, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_FLOW_H_
#define INC_FLOW_H_

#include <stdbool.h>

typedef enum
{
    WAITING_OPTION = 0,
    PUSHBUTTON_TOGGLE = 1,
    TIMER_TOGGLE = 2,
    ADC_READING = 4,
    ADC_LED_TOGGLE = 8,
    ADC_LED_TOGGLE_PWM = 16
} state_t;

void toggle_state(state_t s);
bool is_state_on(state_t s);
void deinit_options(void);
void init_option(state_t s);

#endif /* INC_FLOW_H_ */
