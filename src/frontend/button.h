#ifndef BUTTON_H
#define BUTTON_H

#include "utils.h"
#include <SDL3_ttf/SDL_ttf.h>


typedef struct {
    SDL_FRect rect; //SDL2 data type that defines a rectangle (stores x,y for height and width) with floating point precision
    char *label; //Stores text to be displayed on button
    int is_selected; //Boolean variable tracking state of button
    void (*on_click)(void); //Function pointer that requires takes in a function that has void input/outputs and this function is executed when the button is clicked
} Button;

void draw_button(SDL_Renderer *renderer,TTF_Font *font, Button *button);

#endif