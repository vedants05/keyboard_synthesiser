/*
Integrated Synth with GUI

USAGE:
Build and run this file
- Use the GUI keyboard (click keys) or physical keyboard
- Physical keys: S = A, D = B, F = C, G = D, H = E, J = F, K = G (with sharps on E,T,Y,I,O)
- Click buttons to change waveform and filter
- Use slider to adjust frequency
- ESC to exit

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
