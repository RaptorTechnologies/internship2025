/*
 * graph.h
 *
 *  Created on: Sep 12, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_GRAPH_H_
#define INC_GRAPH_H_

#include <stdint.h>

typedef enum
{
    MODE_BAR,
    MODE_POINT,
} graph_display_mode_t;

void graph_init(uint32_t x_samples, uint32_t y_samples, char *x_axis_title, char *y_axis_title, char *title);
void graph_draw_axis(void);
void graph_update_x_value(uint32_t x, uint32_t y);
void graph_change_mode(graph_display_mode_t display_mode);

#endif /* INC_GRAPH_H_ */
