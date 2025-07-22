/*
 * flow.h
 *
 *  Created on: Jul 21, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_FLOW_H_
#define INC_FLOW_H_

#include <stdbool.h>

typedef enum state {
	WAITING_OPTION = 0,
	PUSHBUTTON_TOGGLE = 1,
	TIMER_TOGGLE = 2,
	ADC_READING = 3,
	ADC_LED_TOGGLE = 4,
	ADC_LED_TOGGLE_PWM = 5
} state_t;

void start_option(state_t num);
state_t get_state(void);
void stop_options(void);

#endif /* INC_FLOW_H_ */
