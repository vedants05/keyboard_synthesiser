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


// Helper function to update button selection states for waveform buttons
static void update_waveform_button_states(Button buttons[], int selected_index) {
    for (int i = 0; i <= 2; i++) {
        buttons[i].is_selected = (i == selected_index);
    }
}

// Helper function to update button selection states for filter buttons
static void update_filter_button_states(Button buttons[], int selected_index) {
    for (int i = 3; i <= 5; i++) {
        buttons[i].is_selected = (i == selected_index);
    }
}

// Helper function to handle waveform selection
static void handle_waveform_selection(waveform wave_type, Button buttons[], int button_index) {
    set_synth_waveform(wave_type);
    update_waveform_button_states(buttons, button_index);
    
    const char* wave_names[] = {"Sine", "Square", "Saw"};
    printf("%s wave selected.\n", wave_names[button_index]);
}

// Helper function to handle filter cycling
static void cycle_filter_type(Button buttons[]) {
    filter_type current = get_current_filter_type();
    switch (current) {
        case GUI_NO_FILTER:
            set_synth_filter_type(GUI_LOW_FILTER);
            update_filter_button_states(buttons, 4);
            printf("Low pass filter selected\n");
            break;
        case GUI_LOW_FILTER:
            set_synth_filter_type(GUI_HIGH_FILTER);
            update_filter_button_states(buttons, 5);
            printf("High pass filter selected\n");
            break;
        case GUI_HIGH_FILTER:
            set_synth_filter_type(GUI_NO_FILTER);
            update_filter_button_states(buttons, 3);
            printf("No filter selected\n");
            break;
    }
}

// Helper function to handle filter cutoff adjustment
static void adjust_filter_cutoff(SynthContext *ctx, float adjustment) {
    if (ctx && ctx->filter) {
        float new_cutoff = ctx->filter->cutoff + adjustment;
        if (adjustment < 0 && new_cutoff <= 500.0f) {
            return; // Don't go below minimum
        }
        set_filter_cutoff(ctx->filter, new_cutoff);
        printf("Filter cutoff %s to %.1f Hz\n", 
               adjustment > 0 ? "increased" : "decreased", 
               ctx->filter->cutoff);
    }
}

// Helper function to handle mouse button interactions with buttons
static void handle_button_interactions(SDL_Point mouse_point, Button buttons[]) {
    // Handle waveform buttons (0-2)
    for (int i = 0; i < 3; i++) {
        SDL_Rect button_rect = {
            (int)buttons[i].rect.x, (int)buttons[i].rect.y,
            (int)buttons[i].rect.w, (int)buttons[i].rect.h
        };

        if (SDL_PointInRect(&mouse_point, &button_rect)) {
            waveform wave_types[] = {GUI_SINE, GUI_SQUARE, GUI_SAW};
            handle_waveform_selection(wave_types[i], buttons, i);
            return;
        }
    }

    // Handle filter buttons (3-5)
    for (int i = 3; i < 6; i++) {
        SDL_Rect button_rect = {
            (int)buttons[i].rect.x, (int)buttons[i].rect.y,
            (int)buttons[i].rect.w, (int)buttons[i].rect.h
        };
        
        if (SDL_PointInRect(&mouse_point, &button_rect)) {
            filter_type filter_types[] = {GUI_NO_FILTER, GUI_LOW_FILTER, GUI_HIGH_FILTER};
            set_synth_filter_type(filter_types[i - 3]);
            update_filter_button_states(buttons, i);
            return;
        }
    }

    //octave controls 
    for (int i = 6; i < NO_OF_BUTTONS; i++) { // number of octave controls
        SDL_Rect button_rect = {
            (int)buttons[i].rect.x, (int)buttons[i].rect.y,
            (int)buttons[i].rect.w, (int)buttons[i].rect.h
        };
        if (SDL_PointInRect(&mouse_point, &button_rect)) {
            // Handle octave buttons 
            if (i == 6) {
                change_octave_down();
            } else {
                change_octave_up();
            }
            // Update is_selected state for octave buttons
            for (int j = 6; j <= 7; j++) {
                buttons[j].is_selected = (i == j);
            }
        }
    }
}

// Helper function to render all GUI elements
static void render_gui(SDL_Renderer *renderer, TTF_Font *font, Slider *filter_slider, Button buttons[]) {
    SDL_SetRenderDrawColor(renderer, 0x22, 0x22, 0x22, 0xFF); // Darker grey background
    SDL_RenderClear(renderer);

    // Draw GUI elements
    draw_slider(renderer, font, filter_slider);

    for (int i = 0; i < NO_OF_BUTTONS; i++) {
        if (i > 5){
            TTF_Font *font = TTF_OpenFont("../src/frontend/fonts/arial.ttf", 40);
            draw_button(renderer,font, &buttons[i]);
        } else {
            draw_button(renderer,font, &buttons[i]);
        }
    }

    // Draw the keyboard
    draw_keyboard(renderer);

    // Update the screen
    SDL_RenderPresent(renderer);
}

void run(SDL_Renderer *renderer, SynthContext *ctx){
    Slider filter_slider;

    TTF_Font *font = TTF_OpenFont("../src/frontend/fonts/arial.ttf", 20);

    //Creating a button list
    Button buttons[NO_OF_BUTTONS];


    add_GUI_elements(&filter_slider, buttons);

    SDL_Event e;
    int quit = 0;
    float step_size_pixels = filter_slider.track_rect.h * 0.025f;
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
                        handle_waveform_selection(GUI_SINE, buttons, 0);
                        break;
                    case SDLK_2:
                        handle_waveform_selection(GUI_SQUARE, buttons, 1);
                        break;
                    case SDLK_3:
                        handle_waveform_selection(GUI_SAW, buttons, 2);
                        break;
                    case SDLK_4:
                        cycle_filter_type(buttons);
                        break;
                    case SDLK_UP:
                        update_slider_from_key(&filter_slider, -1, step_size_pixels);
                        adjust_filter_cutoff(ctx, 500.0f);
                        break;
                    case SDLK_DOWN:
                        update_slider_from_key(&filter_slider, 1, step_size_pixels);
                        adjust_filter_cutoff(ctx, -500.0f);
                        break;
                    case SDLK_LEFT:
                        buttons[6].is_selected = 1;
                        buttons[7].is_selected = 0;
                        change_octave_down();
                        break;
                    case SDLK_RIGHT:
                        buttons[6].is_selected = 0;
                        buttons[7].is_selected = 1;
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

                    // Handle button interactions using helper function
                    handle_button_interactions(mouse_point, buttons);
                    
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
            render_gui(renderer, font, &filter_slider, buttons);
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
