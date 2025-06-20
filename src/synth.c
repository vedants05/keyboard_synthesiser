/*
This is the main file
USAGE:
Build and run this file
S = A and K = G, with all the notes in between, including sharps and flats

] = octave up 
[ = octave down

Rest of the controls are in the GUI
*/

#include <stdlib.h>
#include <stdio.h>

#include "synth.h"
#include "run.h"

int main(void) {
    printf("Starting Integrated Synth with GUI...\n");
    
    // Use the GUI main function which now includes audio backend
    int result = gui_main(0, NULL);
    
    if (result != 0) {
        printf("Program terminated with error\n");
        return 1;
    }

    printf("Program terminated successfully\n");
    return 0;
}
