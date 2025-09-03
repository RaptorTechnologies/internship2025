/*
 * wm8994.h
 *
 *  Created on: Sep 2, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_WM8994_H_
#define INC_WM8994_H_

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"

#define STM32429I_EVAL_WM8994_I2C_ADDR 0b0011010

typedef struct
{
    I2C_HandleTypeDef *hi2c;
    uint16_t addr;
    void const (*error_callback)(void);
} wm8994_t;

typedef enum
{
    HEADPHONE_OUTPUT,
    HEADPHONE_MIC,
} wm8994_output_t;

int wm8994_init_driver(wm8994_t *w, I2C_HandleTypeDef *hi2c, uint16_t addr,
                       void (*error_callback)(void));

#endif /* INC_WM8994_H_ */
