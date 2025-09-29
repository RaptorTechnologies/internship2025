/*
 * audio_processor.c
 *
 *  Created on: Sep 2, 2025
 *      Author: Andrei Trif
 */

#include "audio_processor.h"
#include "arm_math.h"
#include "audio_buffers.h"
#include "graph.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 128
static int16_t raw_buffs[3][BUFF_SIZE * 2];
static int16_t fft_buff[BUFF_SIZE * 4];

// We want to hold two buffers worth of data, so we get a 50% overlap
static int16_t frame[BUFF_SIZE * 2];
static int16_t raw_prev_buff[BUFF_SIZE];
static int16_t prev_buff[BUFF_SIZE];

static int16_t graph_buff[BUFF_SIZE * 3];

static buffs_t buffs;
static SAI_HandleTypeDef *hsai_tx;
static SAI_HandleTypeDef *hsai_rx;
static arm_rfft_instance_q15 fft_inst;
static arm_rfft_instance_q15 fft_inst_inv;

// This window is actually sqrt(hann(256)) so we can use a 50% overlap
const q15_t hann[256] = {
    0,     402,   804,   1206,  1608,  2009,  2410,  2811,  3212,  3612,  4011,  4410,  4808,
    5205,  5602,  5998,  6393,  6786,  7179,  7571,  7962,  8351,  8739,  9126,  9512,  9896,
    10278, 10659, 11039, 11417, 11793, 12167, 12539, 12910, 13279, 13645, 14010, 14372, 14732,
    15090, 15446, 15800, 16151, 16499, 16846, 17189, 17530, 17869, 18204, 18537, 18868, 19195,
    19519, 19841, 20159, 20475, 20787, 21096, 21403, 21705, 22005, 22301, 22594, 22884, 23170,
    23452, 23731, 24007, 24279, 24547, 24811, 25072, 25329, 25582, 25832, 26077, 26319, 26556,
    26790, 27019, 27245, 27466, 27683, 27896, 28105, 28310, 28510, 28706, 28898, 29085, 29268,
    29447, 29621, 29791, 29956, 30117, 30273, 30424, 30571, 30714, 30852, 30985, 31113, 31237,
    31356, 31470, 31580, 31685, 31785, 31880, 31971, 32057, 32137, 32213, 32285, 32351, 32412,
    32469, 32521, 32567, 32609, 32646, 32678, 32705, 32728, 32745, 32757, 32765, 32767, 32765,
    32757, 32745, 32728, 32705, 32678, 32646, 32609, 32567, 32521, 32469, 32412, 32351, 32285,
    32213, 32137, 32057, 31971, 31880, 31785, 31685, 31580, 31470, 31356, 31237, 31113, 30985,
    30852, 30714, 30571, 30424, 30273, 30117, 29956, 29791, 29621, 29447, 29268, 29085, 28898,
    28706, 28510, 28310, 28105, 27896, 27683, 27466, 27245, 27019, 26790, 26556, 26319, 26077,
    25832, 25582, 25329, 25072, 24811, 24547, 24279, 24007, 23731, 23452, 23170, 22884, 22594,
    22301, 22005, 21705, 21403, 21096, 20787, 20475, 20159, 19841, 19519, 19195, 18868, 18537,
    18204, 17869, 17530, 17189, 16846, 16499, 16151, 15800, 15446, 15090, 14732, 14372, 14010,
    13645, 13279, 12910, 12539, 12167, 11793, 11417, 11039, 10659, 10278, 9896,  9512,  9126,
    8739,  8351,  7962,  7571,  7179,  6786,  6393,  5998,  5602,  5205,  4808,  4410,  4011,
    3612,  3212,  2811,  2410,  2009,  1608,  1206,  804,   402
};

static proc_display_t display = DISPLAY_FFT;
static volatile proc_display_t future_display = DISPLAY_FFT;

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

    if (arm_rfft_init_q15(&fft_inst, BUFF_SIZE * 2, 0, 1) != ARM_MATH_SUCCESS)
    {
        Error_Handler();
    }

    if (arm_rfft_init_q15(&fft_inst_inv, BUFF_SIZE * 2, 1, 1) != ARM_MATH_SUCCESS)
    {
        Error_Handler();
    }

    audio_proc_set_display(display);
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

// We actually change the display type only in the next event loop so we don't delay interrupts
void audio_proc_set_display(proc_display_t disp)
{
    future_display = disp;
}

void _audio_proc_set_display(proc_display_t disp)
{
    switch (disp)
    {
        case DISPLAY_FFT:
        {
            graph_change_mode(MODE_BAR);
            graph_init(BUFF_SIZE / 2, (1 << 17) - 1, "Frequency bins", "Amplitude", "FFT");
            graph_draw_axis();
            break;
        }
        case DISPLAY_HP_DATA:
        {
            graph_change_mode(MODE_POINT);
            graph_init(BUFF_SIZE, (1 << 17) - 1, "Time", "Amplitude", "Headphones");
            graph_draw_axis();
            break;
        }
        case DISPLAY_MIC_DATA:
        {
            graph_change_mode(MODE_POINT);
            graph_init(BUFF_SIZE, (1 << 17) - 1, "Time", "Amplitude", "Microphone");
            graph_draw_axis();
            break;
        }
        default:
        {
            return;
        }
    }

    display = disp;
}

proc_display_t audio_proc_get_display(void)
{
    return display;
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

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
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
        // Build the frame using the previous window and the current window
        memcpy(frame, raw_prev_buff, BUFF_SIZE * 2);
        memcpy(frame + BUFF_SIZE, buffs.fft, BUFF_SIZE * 2);
        memcpy(raw_prev_buff, buffs.fft, BUFF_SIZE * 2);

        memcpy(graph_buff, frame, BUFF_SIZE * 2);

        // Apply hann window to frame
        arm_mult_q15(frame, hann, frame, BUFF_SIZE * 2);

        arm_rfft_q15(&fft_inst, frame, fft_buff);

        // DC and Nyquist
        int tmp1 = fft_buff[0];
        int tmp2 = fft_buff[BUFF_SIZE * 2];
        fft_buff[0] = 0;
        fft_buff[BUFF_SIZE * 2] = 0;

        int s = shift * 4;
        if (s > 0)
        {
            memmove(fft_buff + s, fft_buff, 2 * (BUFF_SIZE * 2 - s));
            for (int i = 0; i < s; ++i)
            {
                fft_buff[i] = 0;
            }
        }
        else if (s < 0)
        {
            s = -s;
            memmove(fft_buff, fft_buff + s, 2 * (BUFF_SIZE * 2 - s));
            for (int i = BUFF_SIZE * 2 - s; i < BUFF_SIZE * 2; ++i)
            {
                fft_buff[i] = 0;
            }
        }

        fft_buff[0] = tmp1;
        fft_buff[1] = 0;
        fft_buff[BUFF_SIZE * 2] = tmp2;
        fft_buff[BUFF_SIZE * 2 + 1] = 0;

        // Move frequency magnitudes to graph buffer
        arm_cmplx_mag_squared_q15(fft_buff, graph_buff + BUFF_SIZE, BUFF_SIZE);

        arm_rfft_q15(&fft_inst_inv, fft_buff, frame);

        // Scale up rfft output
        arm_shift_q15(frame, 7, frame, BUFF_SIZE * 2);

        // Apply hann again to prevent saturating the ends of the buffer
        arm_mult_q15(frame, hann, frame, BUFF_SIZE * 2);

        // Get the current window by adding to the previous window
        arm_add_q15(frame, prev_buff, buffs.fft, BUFF_SIZE);
        memcpy(prev_buff, frame + BUFF_SIZE, BUFF_SIZE * 2);

        memcpy(graph_buff + BUFF_SIZE * 2, buffs.fft, BUFF_SIZE * 2);

        // Headphones take stereo data, so we duplicate each value in our fft
        // buffer
        for (int i = BUFF_SIZE - 1; i >= 0; --i)
        {
            buffs.fft[2 * i] = buffs.fft[i];
            buffs.fft[2 * i + 1] = buffs.fft[i];
        }

        buffs_flush(&buffs);

        if (future_display != display)
        {
            _audio_proc_set_display(future_display);
        }

        switch (display)
        {
            case DISPLAY_FFT:
            {
                for (int i = 0; i < BUFF_SIZE / 2; ++i)
                {
                    graph_update_x_value(i, graph_buff[BUFF_SIZE + i] << 7);
                }
                break;
            }
            case DISPLAY_HP_DATA:
            {
                for (int i = 0; i < BUFF_SIZE; ++i)
                {
                    graph_update_x_value(i, (uint32_t)graph_buff[BUFF_SIZE * 2 + i] +
                                                ((1 << 17) - 1) / 2);
                }
                break;
            }
            case DISPLAY_MIC_DATA:
            {
                for (int i = 0; i < BUFF_SIZE; ++i)
                {
                    graph_update_x_value(i, (uint32_t)graph_buff[i] + ((1 << 17) - 1) / 2);
                }
                break;
            }
        }
    }
}
