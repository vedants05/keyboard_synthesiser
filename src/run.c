#include <stdio.h>
#include <stdlib.h>

#include "run.h"
#include <SDL3/SDL_main.h>
#include "frontend/button.h"
#include "frontend/slider.h"
#include "frontend/keyboard.h"
#include "frontend/initialise.h"
#include "frontend/utils.h"
#include "backend/audio.h"
#include "backend/utils.h"
#include "backend/filters.h"

void run(SDL_Renderer *renderer, SynthContext *ctx){
    Slider filter_slider;

    TTF_Font *font = TTF_OpenFont("../src/frontend/fonts/arial.ttf", 20);

    //Creating a button list
    Button buttons[NO_OF_BUTTONS];


    add_GUI_elements(&filter_slider, buttons);

    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = 1;
            }
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                // Handle special function keys first
                switch (e.key.key) {
                    case SDLK_ESCAPE:
                        quit = 1;
                        break;
                    case SDLK_1:
                        set_synth_waveform(GUI_SINE);
                        buttons[0].is_selected = 1;
                        buttons[1].is_selected = 0;
                        buttons[2].is_selected = 0;
                        printf("Sine wave selected.\n");
                        break;
                    case SDLK_2:
                        set_synth_waveform(GUI_SQUARE);
                        buttons[0].is_selected = 0;
                        buttons[1].is_selected = 1;
                        buttons[2].is_selected = 0;
                        printf("Square wave selected.\n");
                        break;
                    case SDLK_3:
                        set_synth_waveform(GUI_SAW);
                        buttons[0].is_selected = 0;
                        buttons[1].is_selected = 0;
                        buttons[2].is_selected = 1;
                        printf("Saw wave selected.\n");
                        break;
                    case SDLK_4: {
                        // Cycle through filters
                        filter_type current = get_current_filter_type();
                        switch (current) {
                            case GUI_NO_FILTER:
                                set_synth_filter_type(GUI_LOW_FILTER);
                                buttons[3].is_selected = 0;
                                buttons[4].is_selected = 1;
                                buttons[5].is_selected = 0;
                                printf("Low pass filter selected\n");
                                break;
                            case GUI_LOW_FILTER:
                                set_synth_filter_type(GUI_HIGH_FILTER);
                                buttons[3].is_selected = 0;
                                buttons[4].is_selected = 0;
                                buttons[5].is_selected = 1;
                                printf("High pass filter selected\n");
                                break;
                            case GUI_HIGH_FILTER:
                                set_synth_filter_type(GUI_NO_FILTER);
                                buttons[3].is_selected = 1;
                                buttons[4].is_selected = 0;
                                buttons[5].is_selected = 0;
                                printf("No filter selected\n");
                                break;
                        }
                        break;
                    }
                    case SDLK_UP:
                        if (ctx && ctx->filter) {
                            set_filter_cutoff(ctx->filter, ctx->filter->cutoff + 500.0f);
                            printf("Filter cutoff increased to %.1f Hz\n", ctx->filter->cutoff);
                        }
                        break;
                    case SDLK_DOWN:
                        if (ctx && ctx->filter && ctx->filter->cutoff > 500.0f) {
                            set_filter_cutoff(ctx->filter, ctx->filter->cutoff - 500.0f);
                            printf("Filter cutoff decreased to %.1f Hz\n", ctx->filter->cutoff);
                        }
                        break;
                    case SDLK_LEFTBRACKET:
                        change_octave_down();
                        break;
                    case SDLK_RIGHTBRACKET:
                        change_octave_up();
                        break;
                    default:
                        // Handle piano keys only if not a special function key
                        if (e.key.repeat == 0) {
                            handle_physical_key_down(e.key.scancode);
                        }
                        break;
                }
            } 
            else if (e.type == SDL_EVENT_KEY_UP) {
                handle_physical_key_up(e.key.scancode);
            }
            else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                // Check slider interactions
                // Check slider interactions
                // ... inside SDL_EVENT_MOUSE_BUTTON_DOWN ...
                if (e.button.button == SDL_BUTTON_LEFT) {
                    // Create an SDL_Point from mouse coordinates
                    SDL_Point mouse_point = {(int)e.button.x, (int)e.button.y};

                    // For sliders and buttons, convert their FRect to Rect for the check
                    SDL_Rect filter_handle_rect_int = {(int)filter_slider.handle_rect.x, (int)filter_slider.handle_rect.y,
                                                    (int)filter_slider.handle_rect.w, (int)filter_slider.handle_rect.h};

                    if (SDL_PointInRect(&mouse_point, &filter_handle_rect_int)) {
                        filter_slider.is_dragging = 1;
                    }


                    for (int i = 0; i < NO_OF_BUTTONS/2; i++) {
                        SDL_Rect button_rect = {
                            (int)buttons[i].rect.x, (int)buttons[i].rect.y,
                            (int)buttons[i].rect.w, (int)buttons[i].rect.h
                        };

                        if (SDL_PointInRect(&mouse_point, &button_rect)) {
                            // Handle waveform buttons (0 = Sine, 1 = Square, 2 = Saw)
                            if (i == 0) {
                                set_synth_waveform(GUI_SINE);
                            } else if (i == 1) {
                                set_synth_waveform(GUI_SQUARE);
                            } else if (i == 2) {
                                set_synth_waveform(GUI_SAW);
                            }

                            // Update is_selected state for waveform buttons
                            for (int j = 0; j <= 2; ++j) {
                                buttons[j].is_selected = (i == j);
                            }
                        }
                    }

                    for (int i = 3; i < NO_OF_BUTTONS; i++) {
                        SDL_Rect button_rect = {
                            (int)buttons[i].rect.x, (int)buttons[i].rect.y,
                            (int)buttons[i].rect.w, (int)buttons[i].rect.h
                        };
                        if (SDL_PointInRect(&mouse_point, &button_rect)) {
                            // Handle filter buttons (3 = No Filter, 4 = Low, 5 = High)
                            if (i == 3) {
                                set_synth_filter_type(GUI_NO_FILTER);
                            } else if (i == 4) {
                                set_synth_filter_type(GUI_LOW_FILTER);
                            } else if (i == 5) {
                                set_synth_filter_type(GUI_HIGH_FILTER);
                            }


                            // Update is_selected state for filter buttons
                            for (int j = 3; j <= 5; j++) {
                                buttons[j].is_selected = (i == j);
                            }
                        }
                    }

                    // Handle keyboard presses (if not dragging a slider)
                    if (!filter_slider.is_dragging ) {
                        handle_key_press(e.button.x, e.button.y); // mouse_x, mouse_y are ints already
                    }
                }

            } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    // Release any dragging sliders
                    filter_slider.is_dragging = 0;
                    // Handle keyboard release
                    handle_key_release(e.button.x, e.button.y);
                }
            } else if (e.type == SDL_EVENT_MOUSE_MOTION) {
                if (filter_slider.is_dragging) {
                    update_slider_from_mouse(&filter_slider, e.motion.y);
                    // Update the filter cutoff based on slider value
                    if (ctx && ctx->filter) {
                        float actual_value = filter_slider.min_val + (filter_slider.value * (filter_slider.max_val - filter_slider.min_val));
                        set_filter_cutoff(ctx->filter, actual_value);
                        printf("Filter cutoff set to %.1f Hz\n", actual_value);
                    }
                }
            }
             // --- Rendering ---
            SDL_SetRenderDrawColor(renderer, 0x22, 0x22, 0x22, 0xFF); // Darker grey background
            SDL_RenderClear(renderer);

            // Draw GUI elements
            draw_slider(renderer, font, &filter_slider);

            for (int i = 0; i < NO_OF_BUTTONS; i++) {
                draw_button(renderer,font, &buttons[i]);
            }
            

            // Draw the keyboard
            draw_keyboard(renderer);

            // Update the screen
            SDL_RenderPresent(renderer);
        }
    }
}


int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    // Initialize SDL for both video and audio
    if (init_SDL() != 0) return 1;
    if (init_window(&window) != 0) return 1;
    if (init_renderer(window, &renderer) != 0) return 1;

    // Initialize audio backend
    SynthContext *ctx = malloc(sizeof(SynthContext));
    if (!ctx) {
        fprintf(stderr, "Failed to allocate synth context\n");
        cleanup(window, renderer);
        return 1;
    }

    ctx->frequency = 440.0;
    ctx->current_amplitude = 0;  // Start silent
    ctx->max_amplitude = 3000;
    ctx->phase = 0;
    ctx->wave_type = SINE;

    SDL_AudioSpec spec = {
        .freq = 48000,
        .format = SDL_AUDIO_S16,
        .channels = 1
    };
    ctx->format = spec.format;
    ctx->channels = spec.channels;
    ctx->sample_rate = spec.freq;

    // Allocate and initialize filter
    ctx->filter = malloc(sizeof(Biquad));
    if (!ctx->filter) {
        fprintf(stderr, "Failed to allocate filter memory\n");
        free(ctx);
        cleanup(window, renderer);
        return 1;
    }
    init_synth_filter(ctx->filter, 48000, LPF, 2000.0f, 0.707f);

    // Connect frontend to backend
    init_audio_backend(ctx);

    // Create audio stream
    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &spec,
        synth_callback,
        ctx
    );

    if (!stream) {
        fprintf(stderr, "Failed to open audio stream: %s\n", SDL_GetError());
        free(ctx->filter);
        free(ctx);
        cleanup(window, renderer);
        return 1;
    }

    SDL_ResumeAudioStreamDevice(stream);

    // Run the GUI
    run(renderer, ctx);

    // Cleanup
    SDL_DestroyAudioStream(stream);
    free(ctx->filter);
    free(ctx);
    cleanup(window, renderer);

    return 0;
}
