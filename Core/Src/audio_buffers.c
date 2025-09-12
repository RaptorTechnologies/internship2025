#include "audio_buffers.h"

static void swap(void **a, void **b);

void buffs_init(buffs_t *buffs, int16_t *mic, int16_t *fft, int16_t *hp)
{
    buffs->done = 0;
    buffs->mic = mic;
    buffs->fft = fft;
    buffs->hp = hp;
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

static void swap(void **a, void **b)
{
    void *tmp = *a;
    *a = *b;
    *b = tmp;
}
