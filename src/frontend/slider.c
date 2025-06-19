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
void draw_slider(SDL_Renderer *renderer, TTF_Font *font, Slider *slider) {
    // Draw track (grey)
    SDL_SetRenderDrawColor(renderer, 0x88, 0x88, 0x88, 0xFF);
    SDL_RenderFillRect(renderer, &slider->track_rect);

    // Draw handle (white)
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &slider->handle_rect);

    SDL_Color text_colour = {0, 0, 0, 255}; // White text color (RGBA)

    // TODO: Add text label using SDL_ttf
     SDL_Surface *text_surface = TTF_RenderText_Solid(font, slider->label,strlen(slider->label) ,text_colour);
    if (text_surface == NULL) {
        // FIX 2: Use SDL_LogMessage for proper SDL3 logging with categories/priorities.
        // TTF_GetError() is the correct function to get SDL_ttf specific error messages.
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                       "Failed to render text surface: %s", SDL_GetError());
        return;
    }

    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    // FIX 3: SDL_FreeSurface has been renamed to SDL_DestroySurface in SDL3.
    SDL_DestroySurface(text_surface); // Free the surface immediately after creating the texture
    // Set to NULL to prevent dangling pointer if you keep 'text_surface' around
    text_surface = NULL;

    if (!text_texture) {
        // FIX 4: Use SDL_LogMessage for proper SDL3 logging.
        // SDL_GetError() is the correct function to get core SDL error messages.
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                       "Failed to create texture from text: %s", SDL_GetError());
        return;
    }

    float text_width_f, text_height_f; // Use floats for the new function
    if (!SDL_GetTextureSize(text_texture, &text_width_f, &text_height_f)) {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                       "Failed to get texture size: %s", SDL_GetError());
        SDL_DestroyTexture(text_texture); // Clean up before returning
        return;
    }

    // Convert float dimensions to int for centering calculation if needed,
    // or keep them as floats for direct use in SDL_FRect.
    int text_width = (int)text_width_f;
    int text_height = (int)text_height_f;

    // 2. Center the text in the slider using SDL_FRect
    SDL_FRect text_rect = {
        slider->handle_rect.x + (slider->handle_rect.w - text_width_f) / 2.0f,
        slider->handle_rect.y + (slider->handle_rect.h - text_height_f) / 2.0f,
        text_width_f,
        text_height_f
    };

    // FIX 2: SDL_RenderCopy has been renamed to SDL_RenderTexture in SDL3.
    SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);

    // 3. Destroy the text texture after rendering
    SDL_DestroyTexture(text_texture);
    text_texture = NULL;
}