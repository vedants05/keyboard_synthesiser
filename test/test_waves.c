#include <SDL3/SDL.h>
#include <math.h>
#include "../src/audio.h"
#include "../src/utils.h"
#include "../src/waves.h"
#include <stdlib.h>


void test_callback(
    void *userdata, // the data passed in
    SDL_AudioStream *stream, // the audio stream
    int additional_amount, // the number of *bytes* required
    int total_amount // how many bytes in total are expected to be queued into the stream in this call or soon.
) {
    SynthContext *ctx = (SynthContext *)userdata;
    int num_samples = additional_amount / sizeof(int16_t);
    int16_t *buffer;
    switch (ctx->wave_type) {
        case SINE:
            buffer = generate_sine(ctx, num_samples);
            break;
        case SAW:
            buffer = generate_saw(ctx, num_samples);
            break;
        case SQUARE:
            buffer = generate_square(ctx, num_samples);
            break;
        default: {
            fprintf(stderr, "Wave generation failed\n");
            return;
        }
            
    }

    if (!buffer) {
        fprintf(stderr, "Failed to generate buffer.\n");
        return;
    }

    SDL_PutAudioStreamData(stream, buffer, additional_amount);
    free(buffer);
}


int main(void) {
    if (initialise_sdl() != 0) {
        exit(1);
    }

    SDL_Window *window = SDL_CreateWindow(
        "Keyboard Test - CLICK HERE FIRST", 
        800, 600, 
        SDL_EVENT_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Initialise renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "Renderer creation failed %s\n", SDL_GetError());
        return 1;
    }


    SynthContext *ctx = malloc(sizeof(SynthContext));
    if (!ctx) {
        fprintf(stderr, "Failed to allocate synth context\n");
        exit(1);
    }

    ctx->sample_rate = 48000;
    ctx->frequency = 440.0;
    ctx->amplitude = 3000;
    ctx->phase = 0;
    ctx->wave_type = SINE;

    SDL_AudioSpec spec = {
        .freq = ctx->sample_rate,
        .format = SDL_AUDIO_S16,
        .channels = 1
    };

    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, 
        &spec,
        test_callback, 
        ctx
    );

    if (!stream) {
        fprintf(stderr, "Failed to create audio stream: %s\n", SDL_GetError());
        free(ctx);
        SDL_Quit();
        return 1;
    }

    SDL_ResumeAudioStreamDevice(stream);
    SDL_Delay(2000);  // Play sine for 2 seconds

    SDL_PauseAudioStreamDevice(stream);
    SDL_FlushAudioStream(stream);
    SDL_Delay(500);   // Let the audio drain and ensure pause is perceptible

    ctx->wave_type = SQUARE;
    SDL_ResumeAudioStreamDevice(stream);
    SDL_Delay(2000);  // Play square for 2 seconds

    SDL_PauseAudioStreamDevice(stream);
    SDL_FlushAudioStream(stream);
    SDL_Delay(500);   // Pause before next

    ctx->wave_type = SAW;
    SDL_ResumeAudioStreamDevice(stream);
    SDL_Delay(2000);  // Play saw for 2 seconds

    SDL_PauseAudioStreamDevice(stream);
    SDL_FlushAudioStream(stream);

    free(ctx);
    SDL_Quit();
    return 0;
}