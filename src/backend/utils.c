#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "audio.h"
#include "notes.h"
#include "filters.h"   

#define SAMPLE_RATE 48000


int initialise_sdl() {
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init video failed: %s\n", SDL_GetError());
        return 1;
    }
    return 0;
}

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

    // Apply your biquad filter here BEFORE pushing to SDL
    apply_biquad(ctx->filter, buffer, num_samples);

    int buffer_size = num_samples * bytes_per_sample;
    SDL_PutAudioStreamData(stream, buffer, buffer_size);

    free(buffer);
}

int run_synth(bool *running, SDL_Event *event, user_state_t *user_state) {
    if (!*running) return 0;

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

    SynthContext *ctx = malloc(sizeof(SynthContext));
    if (!ctx) {
        fprintf(stderr, "Failed to allocate synth context\n");
        return 1;
    }

    ctx->frequency = 440.0;
    ctx->amplitude = 3000;
    ctx->phase = 0;
    ctx->wave_type = SINE;

    SDL_AudioSpec spec = {
        .freq = SAMPLE_RATE,
        .format = SDL_AUDIO_S16,
        .channels = 1
    };
    ctx->format = spec.format;
    ctx->channels = spec.channels;
    ctx->sample_rate = spec.freq;

    // Allocate memory for the filter
    ctx->filter = malloc(sizeof(Biquad));
    if (!ctx->filter) {
        fprintf(stderr, "Failed to allocate filter memory\n");
        free(ctx);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialize filter with desired parameters
    init_synth_filter(ctx->filter, SAMPLE_RATE, LPF, 2000.0f, 0.707f);  // example: low-pass at 8kHz Q=0.707

    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &spec,
        synth_callback,
        ctx
    );

    printf("\n=== IMPORTANT ===\n");
    printf("1. Click on the SDL window to give it focus\n");
    printf("2. Then press keys to test\n");
    printf("3. Press ESC to exit\n");
    printf("=================\n\n");

    while (*running) {
        while (SDL_PollEvent(event)) {
            char note[4] = {0};
            switch (event->type) {
                case SDL_EVENT_QUIT:
                    *running = false;
                    break;
                case SDL_EVENT_KEY_DOWN: {
                    switch (event->key.key) {
                        case SDLK_ESCAPE:
                            *running = false;
                            break;
                        case SDLK_S: strcpy(note, "A"); break;
                        case SDLK_E: strcpy(note, "A#"); break;
                        case SDLK_D: strcpy(note, "B"); break;
                        case SDLK_F: strcpy(note, "C"); break;
                        case SDLK_G: strcpy(note, "D"); break;
                        case SDLK_Y: strcpy(note, "D#"); break;
                        case SDLK_H: strcpy(note, "E"); break;
                        case SDLK_J: strcpy(note, "F"); break;
                        case SDLK_I: strcpy(note, "F#"); break;
                        case SDLK_K: strcpy(note, "G"); break;
                        case SDLK_O: strcpy(note, "G#"); break;

                        // Wave type switching
                        case SDLK_1: ctx->wave_type = SINE; printf("Sine wave selected.\n"); break;
                        case SDLK_2: ctx->wave_type = SQUARE; printf("Square wave selected.\n"); break;
                        case SDLK_3: ctx->wave_type = SAW; printf("Saw wave selected.\n"); break;

                        // Example: Change filter cutoff with keys (optional)
                        case SDLK_UP:
                            set_filter_cutoff(ctx->filter, ctx->filter->cutoff + 500.0f);
                            printf("Filter cutoff increased to %.1f Hz\n", ctx->filter->cutoff);
                            break;
                        case SDLK_DOWN:
                            set_filter_cutoff(ctx->filter, ctx->filter->cutoff - 500.0f);
                            printf("Filter cutoff decreased to %.1f Hz\n", ctx->filter->cutoff);
                            break;

                        // Octave switching
                        case SDLK_RIGHTBRACKET: user_state->octave++; break;
                        case SDLK_LEFTBRACKET: user_state->octave--; break;
                        default: break;
                    }

                    if (strlen(note) > 0) {
                        int octave_to_use = user_state->octave;
                        if (strcmp(note, "A") == 0 || strcmp(note, "A#") == 0 || strcmp(note, "B") == 0)
                            octave_to_use -= 1;
                        if (octave_to_use > 8 || octave_to_use < 0) {
                            printf("Cant play note: outside range\n");
                            continue;
                        }
                        snprintf(note, sizeof(note), "%s%d", note, octave_to_use);
                        double freq = note_to_freq(note);
                        ctx->frequency = freq;
                        SDL_ResumeAudioStreamDevice(stream);
                    }
                    break;
                }
                case SDL_EVENT_KEY_UP:
                    SDL_PauseAudioStreamDevice(stream);
                    break;
                case SDL_EVENT_WINDOW_FOCUS_GAINED:
                    printf("Window gained focus - ready for keyboard input!\n");
                    break;
                case SDL_EVENT_WINDOW_FOCUS_LOST:
                    printf("Window lost focus\n");
                    break;
            }
            SDL_Delay(1);
        }
    }

    SDL_DestroyWindow(window);
    SDL_DestroyAudioStream(stream);
    free(ctx->filter);  // Free the filter memory
    free(ctx);
    SDL_Quit();
    return 0;
}