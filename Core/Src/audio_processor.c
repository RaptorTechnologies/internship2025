/*
 * audio_processor.c
 *
 *  Created on: Sep 2, 2025
 *      Author: Andrei Trif
 */

#include "arm_math.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>

// The buffers shift mic -> fft -> hp -> mic
// Once cycled, the processed data is written to fft and the "done" counter is
// reset
typedef struct
{
    // Used by the RX DMA
    int16_t *mic;

    // Being processed
    int16_t *fft;

    // Used by the TX DMA
    int16_t *hp;

    // Number of DMAs that are done
    int done;
} buffs_t;

void swap(void **a, void **b)
{
    void *tmp = *a;
    *a = *b;
    *b = tmp;
}

void buffs_init(buffs_t *buffs, int16_t *b1, int16_t *b2, int16_t *b3)
{
    buffs->done = 0;
    buffs->mic = b1;
    buffs->fft = b2;
    buffs->hp = b3;
}

void buffs_cycle(buffs_t *buffs)
{
    swap((void **)&buffs->mic, (void **)&buffs->fft);
    swap((void **)&buffs->mic, (void **)&buffs->hp);
}

int buffs_get_done(buffs_t *buffs)
{
    return buffs->done;
}

void buffs_done(buffs_t *buffs)
{
    // Either mic or hp has finished sending via DMA
    ++buffs->done;
}

void buffs_flush(buffs_t *buffs)
{
    // done == 0 signifies that the data in fft is done processing
    buffs->done = 0;
}

#define BUFF_SIZE 128
static int16_t raw_buffs[3][BUFF_SIZE * 2];
static int16_t fft_buff[BUFF_SIZE * 2];
static buffs_t buffs;
static SAI_HandleTypeDef *hsai_tx;
static SAI_HandleTypeDef *hsai_rx;
static arm_rfft_instance_q15 fft_inst;
static arm_rfft_instance_q15 fft_inst_inv;

// Shift is set in audio_proc_set_shift which is called in an interrupt
static volatile int16_t shift;

void audio_proc_set_shift(int16_t s)
{
    shift = s;
}

void sai_tx(void)
{
    if (HAL_SAI_Transmit_DMA(hsai_tx, (uint8_t *)buffs.hp, BUFF_SIZE * 2) != HAL_OK)
    {
        Error_Handler();
    }
}

void sai_rx(void)
{
    if (HAL_SAI_Receive_DMA(hsai_rx, (uint8_t *)buffs.mic, BUFF_SIZE) != HAL_OK)
    {
        Error_Handler();
    }
}

void audio_proc_init(SAI_HandleTypeDef *hsai_transmit, SAI_HandleTypeDef *hsai_receive)
{
    buffs_init(&buffs, raw_buffs[0], raw_buffs[1], raw_buffs[2]);
    hsai_tx = hsai_transmit;
    hsai_rx = hsai_receive;

    if (arm_rfft_init_q15(&fft_inst, BUFF_SIZE, 0, 1) != ARM_MATH_SUCCESS)
    {
        Error_Handler();
    }

    if (arm_rfft_init_q15(&fft_inst_inv, BUFF_SIZE, 1, 1) != ARM_MATH_SUCCESS)
    {
        Error_Handler();
    }
}

void audio_proc_start(void)
{
    sai_tx();
    sai_rx();
}

void audio_proc_stop(void)
{
    if (HAL_SAI_Abort(hsai_tx) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_SAI_Abort(hsai_rx) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (buffs_get_done(&buffs) >= 2)
    {
        // We interrupted processing
        Error_Handler();
    }

    if (buffs_get_done(&buffs) == 1)
    {
        // We received mic data and sent hp data so we can cycle
        buffs_cycle(&buffs);
    }
    buffs_done(&buffs);

    sai_tx();
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai) {
    Error_Handler();
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (buffs_get_done(&buffs) >= 2)
    {
        // We interrupted processing
        Error_Handler();
    }

    if (buffs_get_done(&buffs) == 1)
    {
        // We received mic data and sent hp data so we can cycle
        buffs_cycle(&buffs);
    }
    buffs_done(&buffs);

    sai_rx();
}

void audio_proc_process(void)
{
    if (buffs_get_done(&buffs) >= 2)
    {
        arm_rfft_q15(&fft_inst, buffs.fft, fft_buff);

        // FFT scales our output by 1/BUFF_SIZE, so we have to multiply by
        // BUFF_SIZE
        for (int i = 0; i < BUFF_SIZE; ++i)
        {
            if (i % 2 == 0) {
                if (fft_buff[i] * fft_buff[i] <= 1) {
                    fft_buff[i] = 0;
                }
            }
            fft_buff[i] <<= 7;
        }

        int s = shift;
        // The output of the fft has BUFF_SIZE complex samples so we need to
        // shift by twice the shift value
        if (s > 0)
        {
            memmove(fft_buff + s * 2, fft_buff, 2 * (BUFF_SIZE * 2 - s * 2));
            for (int i = 0; i < s * 2; ++i)
            {
                fft_buff[i] = 0;
            }
        }
        else if (s < 0)
        {
            s = -s;
            memmove(fft_buff, fft_buff + s * 2, 2 * (BUFF_SIZE * 2 - s * 2));
            for (int i = BUFF_SIZE * 2 - s * 2; i < BUFF_SIZE * 2; ++i)
            {
                fft_buff[i] = 0;
            }
        }

        arm_rfft_q15(&fft_inst_inv, fft_buff, buffs.fft);

        // Headphones take stereo data, so we duplicate each value in our fft
        // buffer
        for (int i = BUFF_SIZE - 1; i >= 0; --i)
        {
            buffs.fft[2 * i] = buffs.fft[i] << 1;
            buffs.fft[2 * i + 1] = buffs.fft[i] << 1;
        }

        buffs_flush(&buffs);
    }
}
