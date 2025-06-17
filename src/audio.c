#include "waves.h"
#include "audio.h"
#include "state.h"
#include <stdlib.h>

static int16_t *generate_buffer(wave_generator generator, SynthContext *ctx, int num_samples) {
    int16_t *buffer = generator(ctx, num_samples);
    
    return buffer;
}


void synth_callback(void *userdata, SDL_AudioStream *stream,
                    int additional_amount, int total_amount) {

    SynthContext *ctx = (SynthContext *)userdata;

     // Calculate how many samples we need to generate
    // additional_amount is in bytes, each sample is (channels * sizeof(int16_t)) bytes
    int bytes_per_sample = ctx->channels * sizeof(int16_t);
    int num_samples = additional_amount / bytes_per_sample;

    // 1. Figure out the type of wave
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

    int16_t *buffer =  generator(ctx, num_samples);

    if (!buffer) {
        fprintf(stderr, "Failed to generate audio buffer\n");
        return;
    }

    // Push the audio data to the SDL_AudioStream
    SDL_PutAudioStreamData(stream, buffer, additional_amount);

    // Free the allocated buffer
    free(buffer);
}