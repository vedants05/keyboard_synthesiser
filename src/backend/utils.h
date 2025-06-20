#ifndef BACKEND_UTILS_H
#define BACKEND_UTILS_H

#include <SDL3/SDL.h>
#include "state.h"

extern void synth_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount);
    
#endif