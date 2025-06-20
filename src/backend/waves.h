#ifndef WAVES_H
#define WAVES_H

#include "state.h"

#include <stdint.h>

extern int16_t *generate_sine(SynthContext *ctx, int num_samples);
extern int16_t *generate_square(SynthContext *ctx, int num_samples);
extern int16_t *generate_saw(SynthContext *ctx, int num_samples);

// Helpers for Python tests:
extern SynthContext *malloc_synth_context(int   sample_rate,
                                   double frequency,
                                   int    current_amplitude,
                                   int    max_amplitude,
                                   wave_t wave_type);

// Free everything
extern void free_synth_context(SynthContext *ctx);

#endif // WAVES_H