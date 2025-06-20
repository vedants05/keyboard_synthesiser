#ifndef INITIALISE_H
#define INITIALISE_H

#include <SDL3_ttf/SDL_ttf.h>

#include "utils.h"
#include "button.h"
#include "slider.h"
#include "keyboard.h"

int init_SDL(void);
int init_window(SDL_Window **window);
int init_renderer(SDL_Window *window, SDL_Renderer **renderer);
void cleanup(SDL_Window *window, SDL_Renderer *renderer);
void add_GUI_elements(Slider *filter_slider, Button *button_list);

#endif