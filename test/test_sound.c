#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TONE_FREQ 440.0
#define AMPLITUDE 28000
#define DURATION 2

typedef struct {
    int sample_rate;
    double tone_hz;
    int amplitude;
    int phase;
} SynthContext;

void synth_callback(
    void *userdata, // the data passed in
    SDL_AudioStream *stream, // the audio stream
    int additional_amount, // the number of *bytes* required
    int total_amount // how many bytes in total are expected to be queued into the stream in this call or soon.
) {
    SynthContext *ctx = (SynthContext *)userdata;
    int samples = additional_amount / sizeof(int16_t);
    int16_t *buffer = malloc(additional_amount);
    if (!buffer) {
        fprintf(stderr, "Failed to allocate buffer for audio\n");
        return;
    }

    for (int i = 0; i < samples; ++i) {
        double time = (double)(ctx->phase++) / ctx->sample_rate;
        buffer[i] = (int16_t)(ctx->amplitude * sin(2.0 * M_PI * ctx->tone_hz * time));
    }

    SDL_PutAudioStreamData(stream, buffer, additional_amount);
    free(buffer);
}

int main(void) {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        fprintf(stderr, "Audio could not be initialised: %s\n", SDL_GetError());
        exit(1);
    }

    SynthContext *ctx = malloc(sizeof(SynthContext));
    if (!ctx) {
        fprintf(stderr, "Failed to allocate synth context\n");
        exit(1);
    }
    
    ctx->sample_rate = 48000;
    ctx->tone_hz = 440.0;
    ctx->amplitude = 3000;
    ctx->phase = 0;

    // Initialize audio spec with proper parameters
    SDL_AudioSpec spec = {
        .freq = ctx->sample_rate,
        .format = SDL_AUDIO_S16,
        .channels = 1
    };

    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, synth_callback, ctx);

    
    if (!stream) {
        fprintf(stderr, "Failed to create audio stream: %s\n", SDL_GetError());
        free(ctx);
        SDL_Quit();
        return 1;
    }

    SDL_ResumeAudioStreamDevice(stream);
    SDL_Delay(DURATION * 1000);  // Play sound for duration

    SDL_DestroyAudioStream(stream);
    free(ctx);
    SDL_Quit();
    return 0;
}