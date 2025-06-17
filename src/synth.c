/*
This is the main file
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
    
    printf("Running program...\n");
    int r = run_synth(&running, &event, &user_state);

    if (r != 0) {
        return 1; // error already printed
    }

    printf("Program terminated successfully\n");

    return 0;
}