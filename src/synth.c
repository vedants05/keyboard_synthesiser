/*
This is the main file
*/

#include <stdlib.h>


#include "synth.h"
#include "utils.h"


int main(void) {
    // 1. Initialise SDL
    printf("Initialising SDL...\n");
    if (initialise_sdl() != 0) {
        exit(1); // error already printed 
    }    

    // 2. create and run window
    bool running = true;
    SDL_Event event;
    
    int r = run_synth(&running, &event);

    if (r != 0) {
        exit(1); // error already printed
    }


    return 0;
}