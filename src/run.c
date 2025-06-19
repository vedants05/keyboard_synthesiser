#include <stdio.h>
#include <SDL3/SDL_main.h>
#include "frontend/button.h"
#include "frontend/slider.h"
#include "frontend/keyboard.h"
#include "run.h"
#include "frontend/utils.h"

int init_SDL(void){
    // Initialize SDL (Video only, as audio is backend)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "could not initialize SDL3: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
    SDL_Log("TTF_Init failed: %s", SDL_GetError());
    return 1;
    }
    return 0;
}

int init_window(SDL_Window **window){
    // --- Window and Renderer Setup ---
    *window = SDL_CreateWindow(
        "Synth34",
        900, // Width to accommodate keyboard
        600, // Height
        SDL_WINDOW_RESIZABLE
    );
    if (window == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_MaximizeWindow(*window);
    return 0;
}

int init_renderer(SDL_Window *window, SDL_Renderer **renderer){
    *renderer = SDL_CreateRenderer(window, NULL);
    if (*renderer == NULL) {
        fprintf(stderr, "could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    return 0;
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void add_sliders(Slider *filter_slider){
    // Frequency Slider (20Hz to 20000Hz, for finer control from keys)
    init_slider(filter_slider, 800.0f, 100.0f, 30.0f, 400.0f,
                20.0f, 20000.0f, get_current_frequency(), "Frequency",
                (void (*)(float))set_synth_frequency);

    // // Volume Slider (0.0 to 1.0)
    // init_slider(vol_slider, 800.0f, 100.0f, 20.0f, 300.0f,
    //             0.0f, 1.0f, get_current_volume(), "Volume",
    //             (void (*)(float))set_synth_volume);

}

void add_buttons(Button *button_list) {
    Button sine = { {50.0f, 100.0f, 100.0f, 40.0f}, "Sine", 0, NULL };
    Button square = { {160.0f, 100.0f, 100.0f, 40.0f}, "Square", 0, NULL };
    Button saw = { {270.0f, 100.0f, 100.0f, 40.0f}, "Saw", 0, NULL };
    Button no_filter = { {50.0f, 150.0f, 100.0f, 40.0f}, "No filter", 0, NULL };
    Button low_filter = { {160.0f, 150.0f, 100.0f, 40.0f}, "Low filter", 0, NULL };
    Button high_filter = { {270.0f, 150.0f, 100.0f, 40.0f}, "High filter", 0, NULL };

    button_list[0] = sine;
    button_list[1] = square;
    button_list[2] = saw;
    button_list[3] = no_filter;
    button_list[4] = low_filter;
    button_list[5] = high_filter;
}

void add_GUI_elements(Slider *filter_slider, Button *button_list){
    // Adding Sliders 
    add_sliders(filter_slider);
    
    // Adding buttons
    add_buttons(button_list);

    // Set initial waveform button state 
    if (get_current_waveform_type() == SINE) {
        button_list[0].is_selected = 1;
        button_list[1].is_selected  = 0;
        button_list[2].is_selected  = 0;
    } else if (get_current_waveform_type() == SQUARE) {
        button_list[0].is_selected = 0;
        button_list[1].is_selected  = 1;
        button_list[2].is_selected  = 0;
    } else {
        button_list[0].is_selected = 0;
        button_list[1].is_selected  = 0;
        button_list[2].is_selected  = 1;
    }

    // Set initial filter type button state 
    if (get_current_filter_type() == NO_FILTER) {
        button_list[3].is_selected = 1;
        button_list[4].is_selected  = 0;
        button_list[5].is_selected  = 0;
    } else if (get_current_filter_type() == LOW_FILTER) {
        button_list[3].is_selected = 0;
        button_list[4].is_selected  = 1;
        button_list[5].is_selected  = 0;
    } else {
        button_list[3].is_selected = 0;
        button_list[4].is_selected  = 0;
        button_list[5].is_selected  = 1;
    }

    // Initialize the keyboard (placed below other controls)
    init_keyboard(50.0f, 260.0f); // X, Y position for keyboard start
}

void run(SDL_Renderer *renderer){
    Slider filter_slider;
    Button sine_button;
    Button square_button;
    Button saw_button;
    Button no_filter_button;
    Button low_filter_button;
    Button high_filter_button;

    TTF_Font *font = TTF_OpenFont("../src/frontend/fonts/arial.ttf", 20);


    //Creating a button list
    Button buttons[NO_OF_BUTTONS];

    buttons[0] = sine_button;
    buttons[1] = square_button;
    buttons[2] = saw_button;
    buttons[3] = no_filter_button;
    buttons[4] = low_filter_button;
    buttons[5] = high_filter_button;

    add_GUI_elements(&filter_slider, buttons);

    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = 1;
            }
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                // The e.key.repeat flag is true if this is an auto-repeated key press
                // We only want to trigger the note on the first press.
                if (e.key.repeat == 0) {
                    handle_physical_key_down(e.key.scancode);
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
                                set_synth_waveform(SINE);
                            } else if (i == 1) {
                                set_synth_waveform(SQUARE);
                            } else if (i == 2) {
                                set_synth_waveform(SAW);
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
                                set_filter_type(NO_FILTER);
                            } else if (i == 4) {
                                set_filter_type(LOW_FILTER);
                            } else if (i == 5) {
                                set_filter_type(HIGH_FILTER);
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
                }
                } else {
                    // This logic is for highlighting keys on hover, if you wanted that.
                    // For now, it's implicitly handled by `handle_key_press` on click.
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


int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if (init_SDL() != 0) return 1;
    if (init_window(&window) != 0) return 1;
    if (init_renderer(window, &renderer) != 0) return 1;
    run(renderer);
    cleanup(window, renderer);

    return 0;
}