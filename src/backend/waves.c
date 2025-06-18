#include "waves.h"
#include "audio.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <stdlib.h>
#include <math.h>
#include <stdio.h>


int16_t *generate_sine(SynthContext *ctx, int num_samples) {
    // snapshot of ctx
    int amplitude = ctx->amplitude;
    double frequency = ctx->frequency;    
    int start_phase = ctx->phase;

    // REMEMBER TO FREE BUFFER
    int16_t *buffer = malloc(num_samples * sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "Could not allocate audio buffer\n");
        return NULL;
    }

    double phase = ctx->phase;
    double phase_inc = 2.0 * M_PI * ctx->frequency / ctx->sample_rate;

    for (int i = 0; i < num_samples; ++i) {
        buffer[i] = (int16_t)(ctx->amplitude * sin(phase));
        phase += phase_inc;

        // Wrap phase to stay within 0 - 2π
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
    }

    ctx->phase = phase;
    return buffer;
}


int16_t *generate_saw(SynthContext *ctx, int num_samples) {
    int amplitude = ctx->amplitude;
    double frequency = ctx->frequency;
    int start_phase = ctx->phase;

    // REMEMBER TO FREE BUFFER
    int16_t *buffer = malloc(num_samples * sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "Could not allocate audio buffer\n");
        return NULL;
    }

    for (int i = 0; i < num_samples; ++i) {
        double time = (double)(start_phase + i) / ctx->sample_rate;
        double phase = fmod(frequency * time, 1.0); // 0.0 to <1.0
        buffer[i] = (int16_t)(amplitude * (2.0 * phase - 1.0)); // -1.0 to 1.0 range
    }

    ctx->phase = start_phase + num_samples;
    return buffer;
}

int16_t *generate_square(SynthContext *ctx, int num_samples) {
    int amplitude = ctx->amplitude;
    double frequency = ctx->frequency;
    int start_phase = ctx->phase;

    // REMEMBER TO FREE BUFFER
    int16_t *buffer = malloc(num_samples * sizeof(int16_t));
    if (!buffer) {
        fprintf(stderr, "Could not allocate audio buffer\n");
        return NULL;
    }

    for (int i = 0; i < num_samples; ++i) {
        double time = (double)(start_phase + i) / ctx->sample_rate;
        double phase = fmod(frequency * time, 1.0); // range [0.0, 1.0)
        buffer[i] = (phase < 0.5) ? amplitude : -amplitude;
    }

    ctx->phase = start_phase + num_samples;
    return buffer;
}

