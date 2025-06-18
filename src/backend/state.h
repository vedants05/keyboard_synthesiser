#ifndef TYPES_H
#define TYPES_H

#include <SDL3/SDL.h>
#include "filters.h"

typedef enum { SINE, SQUARE, SAW } wave_t;


typedef struct {
    wave_t wave_type;
    int octave;
    int amplitude;
    FilterType filter_type;
    float cutoff;
    float resonance;
} user_state_t; // this is directly affected by the user changing stuff

typedef struct {
    SDL_AudioFormat format;
    int channels;
    int sample_rate;
    double frequency;
    int amplitude;
    double phase;
    wave_t wave_type;
    Biquad *filter;
} SynthContext;

typedef int16_t* (*wave_generator)(SynthContext *ctx, int num_samples);

#endif