#include <stdio.h>
#include <string.h>

#include "button.h"


// Render a button - renderer object is responsible for drawing all items in the window
void draw_button(SDL_Renderer *renderer, TTF_Font *font, Button *button) {
    // 1. Draw the button background
    if (button->is_selected) {
        SDL_SetRenderDrawColor(renderer, 0x00, 0xAA, 0x00, 0xFF); // Green if selected
    } else {
        SDL_SetRenderDrawColor(renderer, 0x44, 0x44, 0x44, 0xFF); // Dark grey if not
    }
    SDL_RenderFillRect(renderer, &button->rect);

    // 2. Draw the button border
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); // White border
    SDL_RenderRect(renderer, &button->rect); // Draws the outline

    // 3. Render and draw the text label
    SDL_Color text_colour = {255, 255, 255, 255}; // White text color (RGBA)

    // FIX 1: TTF_RenderText_Solid does NOT take strlen() as an argument.
    // It expects the C-string pointer directly.
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, button->label,strlen(button->label) ,text_colour);
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

    // 2. Center the text in the button using SDL_FRect
    SDL_FRect text_rect = {
        button->rect.x + (button->rect.w - text_width_f) / 2.0f,
        button->rect.y + (button->rect.h - text_height_f) / 2.0f,
        text_width_f,
        text_height_f
    };

    // FIX 2: SDL_RenderCopy has been renamed to SDL_RenderTexture in SDL3.
    SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);

    // 3. Destroy the text texture after rendering
    SDL_DestroyTexture(text_texture);
    text_texture = NULL;
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