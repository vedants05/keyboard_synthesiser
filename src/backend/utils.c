#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "notes.h"
#include "filters.h"   

#define SAMPLE_RATE 48000

void synth_callback(void *userdata, SDL_AudioStream *stream,
                    int additional_amount, int total_amount) {

    SynthContext *ctx = (SynthContext *)userdata;
    int bytes_per_sample = ctx->channels * sizeof(int16_t);
    int num_samples = additional_amount / bytes_per_sample;

    wave_generator generator;
    switch (ctx->wave_type) {
        case SINE: 
            generator = &generate_sine;
            break;
        case SQUARE: 
            generator = &generate_square;
            break;
        case SAW: 
            generator = &generate_saw;
            break;
        default: 
            fprintf(stderr, "Wave could not be generated\n");
            return;
    }

    int16_t *buffer = generator(ctx, num_samples);
    if (!buffer) {
        fprintf(stderr, "Failed to generate audio buffer\n");
        return;
    }

    if (ctx->filter->type != NO_FILTER) {
        apply_biquad(ctx->filter, buffer, num_samples);
    }

    int buffer_size = num_samples * bytes_per_sample;
    SDL_PutAudioStreamData(stream, buffer, buffer_size);

    free(buffer);
}
