#include <stdio.h>

#include "initialise.h"


int init_SDL(void){
    // Initialize SDL with both video and audio
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        fprintf(stderr, "could not initialize SDL3: %s\n", SDL_GetError());
        return 1;
    }
    if (!TTF_Init()) {
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

void add_GUI_elements(Slider *filter_slider, Button *button_list){
    // Adding Sliders 
    add_sliders(filter_slider);
    
    // Adding buttons
    add_buttons(button_list);

    // Set initial waveform button state 
    if (get_current_waveform_type() == GUI_SINE) {
        button_list[0].is_selected = 1;
        button_list[1].is_selected  = 0;
        button_list[2].is_selected  = 0;
    } else if (get_current_waveform_type() == GUI_SQUARE) {
        button_list[0].is_selected = 0;
        button_list[1].is_selected  = 1;
        button_list[2].is_selected  = 0;
    } else {
        button_list[0].is_selected = 0;
        button_list[1].is_selected  = 0;
        button_list[2].is_selected  = 1;
    }

    // Set initial filter type button state 
    if (get_current_filter_type() == GUI_NO_FILTER) {
        button_list[3].is_selected = 1;
        button_list[4].is_selected  = 0;
        button_list[5].is_selected  = 0;
    } else if (get_current_filter_type() == GUI_LOW_FILTER) {
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

