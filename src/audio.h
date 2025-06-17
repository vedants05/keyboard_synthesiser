#ifndef AUDIO_H
#define AUDIO_H

#include <SDL3/SDL.h>
#include "waves.h"

#define SAMPLE_RATE 48000


// This is called to get the next few samples
extern void synth_callback(void *userdata, SDL_AudioStream *stream,
                    int additional_amount, int total_amount);

#endif
