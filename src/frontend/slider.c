#include <stdio.h>

#include "slider.h"



// Initializing a slider
void init_slider(Slider *slider, float x, float y, float width, float height,
                 float min_val, float max_val, float initial_val, char *label,
                 void (*callback)(float)) {


    slider->track_rect = (SDL_FRect){x, y, width, height};

    // Calculate initial handle position based on initial_val relative to min/max - transforms intital_val into its position on slider between 0 and 1
    float initial_norm_pos = (initial_val - min_val) / (max_val - min_val);
    slider->handle_rect = (SDL_FRect){x - width / 2, y + height * (1.0f - initial_norm_pos) - 10, width * 2, 20.0f}; //Calculating handle position relative to slider track

    slider->value = initial_norm_pos; // Normalized 0.0-1.0
    slider->min_val = min_val;
    slider->max_val = max_val;
    slider->is_dragging = 0;
    slider->on_value_changed = callback;
    slider->label = label;

    // Safety check to ensure that the calculated starting position of the handle is never visually outside the bounds of the track
    if (slider->handle_rect.y < slider->track_rect.y) {
        slider->handle_rect.y = slider->track_rect.y;
    } 
    if (slider->handle_rect.y > slider->track_rect.y + slider->track_rect.h - slider->handle_rect.h) {
        slider->handle_rect.y = slider->track_rect.y + slider->track_rect.h - slider->handle_rect.h;
    }

}

// Update slider value based on mouse Y position if user is dragging the slider
void update_slider_from_mouse(Slider *slider, int mouse_y) {

    //Calulating minimum and maximum possible y coordinates for the top edge of the handle so it can't move off the track
    float handle_y_min = slider->track_rect.y;
    float handle_y_max = slider->track_rect.y + slider->track_rect.h - slider->handle_rect.h;

    //Sets the handle's new vertical position to be centered on the mouse cursor
    slider->handle_rect.y = (float)mouse_y - slider->handle_rect.h / 2.0f;

    // Fixing handle position within slider bounds
    if (slider->handle_rect.y < handle_y_min) {
        slider->handle_rect.y = handle_y_min;
    } else if (slider->handle_rect.y > handle_y_max) {
        slider->handle_rect.y = handle_y_max;
    }

    // Convert handles current y position to the value relative to slider value between 0 and 1
    float normalized_pos = 1.0f - ((slider->handle_rect.y - handle_y_min) / (handle_y_max - handle_y_min));
    slider->value = normalized_pos; // Normalized 0.0-1.0
    //Actual value relative to actual numbers for the slider
    float actual_value = slider->min_val + normalized_pos * (slider->max_val - slider->min_val);

    // If a callback function was assigned to the slider, pass in the updated value
    if (slider->on_value_changed) {
        slider->on_value_changed(actual_value);
    }
}

// Render a slider
void draw_slider(SDL_Renderer *renderer, Slider *slider) {
    // Draw track (grey)
    SDL_SetRenderDrawColor(renderer, 0x88, 0x88, 0x88, 0xFF);
    SDL_RenderFillRect(renderer, &slider->track_rect);

    // Draw handle (white)
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &slider->handle_rect);

    // TODO: Add text label using SDL_ttf
}