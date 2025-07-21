/*
 * flow.h
 *
 *  Created on: Jul 21, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_FLOW_H_
#define INC_FLOW_H_

enum state {
	WAITING_OPTION, OPTION_1, OPTION_2, OPTION_3, OPTION_4
};
extern enum state state;

void start_option(int num);
void stop_options(void);

#endif /* INC_FLOW_H_ */
