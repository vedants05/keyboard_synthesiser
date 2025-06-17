#ifndef UTILS_H
#define UTILS_H

#include <SDL3/SDL.h>
#include "state.h"

extern int initialise_sdl();

extern int run_synth(bool *running, SDL_Event *event, user_state_t *user_state);
    
#endif