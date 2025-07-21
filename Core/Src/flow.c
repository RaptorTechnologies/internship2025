/*
 * flow.c
 *
 *  Created on: Jul 21, 2025
 *      Author: Andrei Trif
 */

#include "flow.h"

enum state state = WAITING_OPTION;

void start_option(int num) {
	state = num;
}

void stop_options(void) {
	state = WAITING_OPTION;
}
