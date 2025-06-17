#ifndef TYPES_H
#define TYPES_H

#include <SDL3/SDL.h>
#include "filters.h"

typedef enum { SINE, SQUARE, SAW } wave_t;

typedef struct {
    SDL_AudioFormat format;
    int channels;
    int sample_rate;
    double frequency;
    int amplitude;
    int phase;
    wave_t wave_type;
    Biquad *filter;
}
SynthContext;

typedef int16_t* (*wave_generator)(SynthContext *ctx, int num_samples);


#endif