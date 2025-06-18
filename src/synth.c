/*
This is the main file

USAGE:
Build and run this file
S = A and K = G, with all the notes in between, including sharps and flats
1 = Sine wave
2 = Square wave
3 = Saw wave
Arrow up = Increase filter cutoff
Arrow down = Decrease filter cutoff
] = octave up 
[ = octave down

*/


#include <stdlib.h>
#include <stdio.h>

#include "synth.h"
#include "backend/utils.h"
#include "backend/state.h"


int main(void) {
    // 1. Initialise SDL
    printf("Initialising SDL...\n");
    if (initialise_sdl() != 0) {
        exit(1); // error already printed 
    }    

    // 2. create and run window
    bool running = true;
    SDL_Event event;
    user_state_t user_state;

    user_state.octave = 4;
    
    printf("Running program...\n");
    int r = run_synth(&running, &event, &user_state);

    if (r != 0) {
        return 1; // error already printed
    }

    printf("Program terminated successfully\n");

    return 0;
}