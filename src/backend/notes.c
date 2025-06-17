#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "notes.h"

// Helper to extract note name and octave from input string
double note_to_freq(const char *note_str) {
    char name[3] = {0}; // for "C" or "C#"
    int octave;

    size_t len = strlen(note_str);
    if (len < 2 || len > 4)
        return -1;

    // Parse note name
    if (note_str[1] == '#') {
        name[0] = toupper(note_str[0]);
        name[1] = '#';
        octave = atoi(&note_str[2]);
    } else {
        name[0] = toupper(note_str[0]);
        octave = atoi(&note_str[1]);
    }

    // Search in note_table
    for (int i = 0; i < NOTE_COUNT; i++) {
        if (strcmp(note_table[i].name, name) == 0 && note_table[i].octave == octave) {
            return note_table[i].freq;
        }
    }

    return -1; // Not found
}

