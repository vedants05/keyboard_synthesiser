#include <stdio.h>

#include "button.h"


// Render a button - renderer object is responsible for drawing all items in the window
void draw_button(SDL_Renderer *renderer, Button *button) {
    if (button->is_selected) {
        SDL_SetRenderDrawColor(renderer, 0x00, 0xAA, 0x00, 0xFF); // Green if selected - The four hex arguments are red, green, blue and alpha
    } else {
        SDL_SetRenderDrawColor(renderer, 0x44, 0x44, 0x44, 0xFF); // Dark grey if not
    }
    SDL_RenderFillRect(renderer, &button->rect); //Draws a solid, filled rectangle from the colour set using the if/else block above 
                                                 //Takes in a pointer to the rect of button which stores height and width of button

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); // White border
    SDL_RenderRect(renderer, &button->rect); // Draws the outline of the rectangle with the colour border selected above

    // TODO: Add text label using SDL_ttf
}
