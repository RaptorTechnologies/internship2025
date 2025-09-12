/*
 * audio_buffers.h
 *
 *  Created on: Sep 12, 2025
 *      Author: Andrei Trif
 */

#ifndef INC_AUDIO_PROCESSOR_H_
#define INC_AUDIO_PROCESSOR_H_

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

void buffs_init(buffs_t *buffs, int16_t *mic, int16_t *fft, int16_t *hp);
void buffs_cycle(buffs_t *buffs);
int buffs_get_done(buffs_t *buffs);
void buffs_done(buffs_t *buffs);
void buffs_flush(buffs_t *buffs);

#endif /* INC_AUDIO_PROCESSOR_H_ */
