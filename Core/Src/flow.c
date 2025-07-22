/*
 * flow.c
 *
 *  Created on: Jul 21, 2025
 *      Author: Andrei Trif
 */

#include "flow.h"

static state_t state = WAITING_OPTION;

void start_option(state_t num) {
	state = num;
}

state_t get_state(void) {
	return state;
}

void stop_options(void) {
	state = WAITING_OPTION;
}
