/*
 * audio_processor.h
 *
 *  Created on: Sep 3, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_AUDIO_PROCESSOR_H_
#define INC_AUDIO_PROCESSOR_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

typedef enum
{
    DISPLAY_FFT,
    DISPLAY_MIC_DATA,
    DISPLAY_HP_DATA
} proc_display_t;

void audio_proc_init(SAI_HandleTypeDef *hsai_transmit, SAI_HandleTypeDef *hsai_receive);
void audio_proc_process(void);
void audio_proc_start(void);
void audio_proc_stop(void);
void audio_proc_set_shift(int16_t s);
void audio_proc_set_display(proc_display_t disp);
proc_display_t audio_proc_get_display(void);

#endif /* INC_AUDIO_PROCESSOR_H_ */
