#ifndef SLIDER_H
#define SLIDER_H

#include "utils.h"
#include <SDL3_ttf/SDL_ttf.h>

// Structure for a generic slider
typedef struct {
    SDL_FRect track_rect; //Size of sliders background track
    SDL_FRect handle_rect; //Size of handle for slider
    float value; // Stores sliders current value as a ratio between 0.0 (bottom) and 1.0 (top)
    float min_val; //Actual min and maximum values the slider represents
    float max_val;
    int is_dragging; //Boolean value to track whether the user is currently holding the mouse down
    void (*on_value_changed)(float); // Function to be called when value of slider changes - takes in a float which represents the current value of slider
    char *label; //Label to be written on slider
} Slider;

void init_slider(Slider *slider, float x, float y, float width, float height, 
                float min_val, float max_val, float initial_val, char *label, void (*callback)(float));
         
void update_slider_from_mouse(Slider *slider, int mouse_y);

void draw_slider(SDL_Renderer *renderer, TTF_Font *font, Slider *slider);

void add_sliders(Slider *filter_slider);

#endif
