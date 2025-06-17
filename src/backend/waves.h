#ifndef WAVES_H
#define WAVES_H

#include "state.h"

#include <stdint.h>

extern int16_t *generate_sine(SynthContext *ctx, int num_samples);
extern int16_t *generate_square(SynthContext *ctx, int num_samples);
extern int16_t *generate_saw(SynthContext *ctx, int num_samples);

#endif // WAVES_H