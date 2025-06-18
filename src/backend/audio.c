#include "waves.h"
#include "audio.h"
#include "state.h"
#include <stdlib.h>
#include <stdio.h>

static int16_t *generate_buffer(wave_generator generator, SynthContext *ctx, int num_samples) {
    int16_t *buffer = generator(ctx, num_samples);
    return buffer;
}

