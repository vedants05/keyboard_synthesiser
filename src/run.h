#ifndef RUN_H
#define RUN_H

#include <SDL3/SDL.h>

extern int init_SDL(void);
int init_window(SDL_Window **window);
int init_renderer(SDL_Window *window, SDL_Renderer **renderer);
void cleanup(SDL_Window *window, SDL_Renderer *renderer);
void add_sliders(Slider *freq_slider, Slider *vol_slider);
void add_buttons(Button *sine_button, Button *square_button, Button *saw_button);
void add_GUI_elements(Slider *freq_slider, Slider *vol_slider,Button *sine_button, Button *square_button, Button *saw_button);
void run(SDL_Renderer *renderer);


#endif