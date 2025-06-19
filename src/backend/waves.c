#include "waves.h"
#include "audio.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <stdlib.h>
#include <math.h>
#include <stdio.h>


int16_t *generate_sine(SynthContext *ctx, int num_samples) {
    // No sound to output
    if (ctx->current_amplitude == 0) {
        int16_t *buffer = calloc(num_samples, sizeof(int16_t));  // Zeroed buffer
        if (!buffer) {
            fprintf(stderr, "Could not allocate silent audio buffer\n");
            return NULL;
        }
        return buffer;
    }

    // snapshot of ctx
    int amplitude = ctx->current_amplitude;
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
        buffer[i] = (int16_t)(ctx->current_amplitude * sin(phase));
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
    // No sound to output
    if (ctx->current_amplitude == 0) {
        int16_t *buffer = calloc(num_samples, sizeof(int16_t));  // Zeroed buffer
        if (!buffer) {
            fprintf(stderr, "Could not allocate silent audio buffer\n");
            return NULL;
        }
        return buffer;
    }

    int amplitude = ctx->current_amplitude;
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

    // No sound to output
    if (ctx->current_amplitude == 0) {
        int16_t *buffer = calloc(num_samples, sizeof(int16_t));  // Zeroed buffer
        if (!buffer) {
            fprintf(stderr, "Could not allocate silent audio buffer\n");
            return NULL;
        }
        return buffer;
    }

    int amplitude = ctx->current_amplitude;
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


// For Python/ctypes unit testing:
SynthContext *malloc_synth_context(int   sample_rate,
                                   double frequency,
                                   int    current_amplitude,
                                   int    max_amplitude,
                                   wave_t wave_type)
{
    SynthContext *ctx = malloc(sizeof(SynthContext));
    if (!ctx) return NULL;

    ctx->format             = SDL_AUDIO_S16;
    ctx->channels           = 1;
    ctx->sample_rate        = sample_rate;
    ctx->frequency          = frequency;
    ctx->current_amplitude  = current_amplitude;
    ctx->max_amplitude      = max_amplitude;
    ctx->phase              = 0.0;
    ctx->wave_type          = wave_type;

    ctx->filter = malloc(sizeof(Biquad));
    if (ctx->filter) {
        init_synth_filter(ctx->filter,
                          sample_rate,
                          LPF,
                          20000.0f,
                          0.707f);
    }
    return ctx;
}

void free_synth_context(SynthContext *ctx)
{
    if (!ctx) return;
    if (ctx->filter) {
        free(ctx->filter);
    }
    free(ctx);
}