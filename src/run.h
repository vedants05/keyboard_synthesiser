#ifndef RUN_H
#define RUN_H

#include <SDL3/SDL.h>
#define NO_OF_BUTTONS 6

extern int init_SDL(void);
int init_window(SDL_Window **window);
int init_renderer(SDL_Window *window, SDL_Renderer **renderer);
void cleanup(SDL_Window *window, SDL_Renderer *renderer);
void add_sliders(Slider *freq_slider);
void add_buttons(Button *buttons);
void add_GUI_elements(Slider *freq_slider, Button *buttons);
void run(SDL_Renderer *renderer);


#endif