/*
 * audio_processor.h
 *
 *  Created on: Sep 3, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_AUDIO_PROCESSOR_H_
#define INC_AUDIO_PROCESSOR_H_

void audio_proc_init(SAI_HandleTypeDef *hsai_transmit, SAI_HandleTypeDef *hsai_receive);
void audio_proc_process(void);
void audio_proc_start(void);
void audio_proc_stop(void);
void audio_proc_set_shift(int16_t s);

#endif /* INC_AUDIO_PROCESSOR_H_ */
