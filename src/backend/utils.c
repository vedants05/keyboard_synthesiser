#include <SDL3/SDL.h>
#include <stdlib.h>
#include "audio.h"
#include <stdio.h>

int initialise_sdl() {
    
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        fprintf(stderr, "Audio subsystem couldn't be initialised: %s\n", SDL_GetError());
        return 1;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Audio subsystem couldn't be initialised: %s\n", SDL_GetError());
        return 1;
    }

    return 0;
}

int run_synth(bool *running, SDL_Event *event) {
    if (!*running) {
        return 0;
    }

    printf("Creating window...\n");
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
    
    // // Initialise renderer
    // SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    // if (!renderer) {
    //     fprintf(stderr, "Renderer creation failed %s\n", SDL_GetError());
    //     return 1;
    // }


    // Initialise audio
    SynthContext *ctx = malloc(sizeof(SynthContext)); // the data that is passed in
    if (!ctx) {
        fprintf(stderr, "Failed to allocate synth context\n");
        return 1;
    } 


    ctx->sample_rate = 48000;
    ctx->frequency = 440.0;
    ctx->amplitude = 3000;
    ctx->phase = 0;
    ctx->wave_type = SINE;

    SDL_AudioSpec spec = {0}; // Zero

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
            printf("The program can run\n");
            *running = false;
            SDL_Delay(16);
        }
    }


    SDL_DestroyWindow(window);
    SDL_DestroyAudioStream(stream);
    SDL_Quit();
}