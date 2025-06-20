#include <stdio.h>
#include <math.h>

#include "keyboard.h"
#include "../backend/notes.h"


// Global flag to indicate if a note is currently playing 
static int note_is_playing = 0;

// Global octave offset - changed with right and left keys
static int octave_offset = 0;

//Key dimensions
const float WHITE_KEY_WIDTH = 80.0f;
const float WHITE_KEY_HEIGHT = 240.0f;
const float BLACK_KEY_WIDTH = 48.0f;
const float BLACK_KEY_HEIGHT = 144.0f;

// Array to hold all keys 
PianoKey keys[NUM_WHITE_KEYS + NUM_BLACK_KEYS]; // One octave (C to C)

// Note names for the keys (white keys: A3, B3, C4, D4, E4, F4, G4, A4)
const char* white_key_notes[NUM_WHITE_KEYS] = {"A3", "B3", "C4", "D4", "E4", "F4", "G4", "A4"};
const char* black_key_notes[NUM_BLACK_KEYS] = {"A#3", "C#4", "D#4", "F#4", "G#4"};

// Map physical keys to virtual piano keys
const SDL_Scancode white_key_scancodes[NUM_WHITE_KEYS] = {
    SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F, SDL_SCANCODE_G,
    SDL_SCANCODE_H, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L
};

const SDL_Scancode black_key_scancodes[NUM_BLACK_KEYS] = {
    SDL_SCANCODE_E, SDL_SCANCODE_T, SDL_SCANCODE_Y, SDL_SCANCODE_I, SDL_SCANCODE_O
};


// Helper function to get note name with octave offset applied
void get_adjusted_note_name(int key_index, char* result) {
    const char* base_note;
    
    // Get the base note name
    if (key_index < NUM_WHITE_KEYS) {
        base_note = white_key_notes[key_index];
    } else {
        base_note = black_key_notes[key_index - NUM_WHITE_KEYS];
    }
    
    // Parsing the base note to extract note name and octave
    char note_name[3] = {0};
    int base_octave;
    
    if (base_note[1] == '#') {
        note_name[0] = base_note[0];
        note_name[1] = '#';
        base_octave = base_note[2] - '0';
    } else {
        note_name[0] = base_note[0];
        base_octave = base_note[1] - '0';
    }
    
    // Apply octave offset
    int adjusted_octave = base_octave + octave_offset;
    
    // Boundary checking
    if (adjusted_octave < 0) adjusted_octave = 0;
    if (adjusted_octave > 8) adjusted_octave = 8;
    
    // Build the result string
    sprintf(result, "%s%d", note_name, adjusted_octave);
}


//Initialize the keyboard - takes in top left coordinates for the keyboard
void init_keyboard(float start_x, float start_y) {
    int key_index = 0; //Tracks keys array for the key structs that are being initialised

    // Initialize white keys (A3, B3, C4, D4, E4, F4, G4, A4)
    float white_key_pos_x = start_x;
    
    for (int i = 0; i < NUM_WHITE_KEYS; i++) { 
        keys[key_index].is_black_key = 0;
        keys[key_index].is_pressed = 0;
        keys[key_index].rect = (SDL_FRect){white_key_pos_x, start_y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT};
        white_key_pos_x += WHITE_KEY_WIDTH;
        keys[key_index].scancode = white_key_scancodes[i];
        key_index++;
    }

    // Initialize black keys (A#3, C#4, D#4, F#4, G#4)
    key_index = NUM_WHITE_KEYS; // Start adding black keys after white keys
    float black_key_offset_x[] = {1.0f, 3.0f, 4.0f, 6.0f, 7.0f}; // Offsets from start of white key

    for (int i = 0; i < NUM_BLACK_KEYS; i++) {
        keys[key_index].is_black_key = 1;
        keys[key_index].is_pressed = 0;
        keys[key_index].rect = (SDL_FRect){start_x + black_key_offset_x[i] * WHITE_KEY_WIDTH - BLACK_KEY_WIDTH / 2,
                                            start_y,
                                            BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT};
        keys[key_index].scancode = black_key_scancodes[i];
        key_index++;
    }
}

//When a key from the physical keyboard is pressed
void handle_physical_key_down(SDL_Scancode scancode) {

    // Stopping a not that is already playing
    if (note_is_playing) {
        stop_synth_note();
        note_is_playing = 0;

        // Reset is_pressed flag for all keys 
        for (int i = 0; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; i++) {
            keys[i].is_pressed = 0;
        }
    }

    for (int i = 0; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; i++) {
        if (keys[i].scancode == scancode) {
            if (!keys[i].is_pressed) {
                keys[i].is_pressed = 1;

                // Get the adjusted note name with octave offset
                char note_name[5];
                get_adjusted_note_name(i, note_name);

                // Calculate frequency using the simple note_to_freq function
                double frequency = note_to_freq(note_name);
                if (frequency > 0) {
                    start_synth_note(frequency, get_current_volume());
                    note_is_playing = 1;
                }
            }
            return; 
        }
    }
}

//When key is released
void handle_physical_key_up(SDL_Scancode scancode) {

    for (int i = 0; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; i++) {
        if (keys[i].scancode == scancode) {
            if (keys[i].is_pressed) {
                keys[i].is_pressed = 0;
                if (note_is_playing) {
                    stop_synth_note();
                    note_is_playing = 0;
                }
            }
            return; 
        }
    }
}

//Handles key press on UI window
void handle_key_press(int mouse_x, int mouse_y) {
    
    // Mouse coordinates
    SDL_Point mouse_point = {mouse_x, mouse_y};

    for (int i = NUM_WHITE_KEYS; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; i++) {

        //Check black keys first because they overlap white keys
        //Get SDL_Rect of a key and compare it to mouse_point
        SDL_Rect key_rect_int = {(int)keys[i].rect.x, (int)keys[i].rect.y,
                                 (int)keys[i].rect.w, (int)keys[i].rect.h};
        if (SDL_PointInRect(&mouse_point, &key_rect_int)) { 
            if (!keys[i].is_pressed) {
                keys[i].is_pressed = 1;
                
                // Get the adjusted note name with octave offset
                char note_name[5];
                get_adjusted_note_name(i, note_name);

                // Calculate frequency using the simple note_to_freq function
                double frequency = note_to_freq(note_name);
                if (frequency > 0) {
                    start_synth_note(frequency, get_current_volume());
                    note_is_playing = 1;
                }
            }
            return;
        }
    }

    for (int i = 0; i < NUM_WHITE_KEYS; i++) {
        SDL_Rect key_rect_int = {(int)keys[i].rect.x, (int)keys[i].rect.y,
                                 (int)keys[i].rect.w, (int)keys[i].rect.h};
        if (SDL_PointInRect(&mouse_point, &key_rect_int)) {
            if (!keys[i].is_pressed) {
                keys[i].is_pressed = 1;
                
                // Get the adjusted note name with octave offset
                char note_name[5];
                get_adjusted_note_name(i, note_name);

                // Calculate frequency using the simple note_to_freq function
                double frequency = note_to_freq(note_name);
                if (frequency > 0) {
                    start_synth_note(frequency, get_current_volume());
                    note_is_playing = 1;
                }
            }
            return;
        }
    }
}

// Function to handle key release
void handle_key_release(int mouse_x, int mouse_y) {

    for (int i = 0; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; i++) {
        if (keys[i].is_pressed) {
            keys[i].is_pressed = 0;
        }
    }
    
    // Tell backend to stop sound
    if (note_is_playing) {
        stop_synth_note();
        note_is_playing = 0;
    }
}

// Functions to change octave
void change_octave_up() {
    if (octave_offset < 3) { // Limit to a range
        octave_offset++;
        printf("Octave up: %+d\n", octave_offset);
    }
}

void change_octave_down() {
    if (octave_offset > -3) { // Limit to a range
        octave_offset--;
        printf("Octave down: %+d\n", octave_offset);
    }
}

// Draw the keyboard
void draw_keyboard(SDL_Renderer *renderer) {
    // Draw white keys first
    for (int i = 0; i < NUM_WHITE_KEYS; ++i) {
        if (!keys[i].is_black_key) { // Ensure it's a white key
            if (keys[i].is_pressed) {
                SDL_SetRenderDrawColor(renderer, 0xCC, 0xCC, 0xCC, 0xFF); // Grey if pressed
            } else {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); // White
            }
            SDL_RenderFillRect(renderer, &keys[i].rect);
            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF); // Black border
            SDL_RenderRect(renderer, &keys[i].rect);
        }
    }

    // Draw black keys second - so they appear on top
    for (int i = NUM_WHITE_KEYS; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; ++i) {
        if (keys[i].is_black_key) { // Ensure it's a black key
            if (keys[i].is_pressed) {
                SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0xFF); // Darker grey if pressed
            } else {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF); // Black
            }
            SDL_RenderFillRect(renderer, &keys[i].rect);
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); // White border
            SDL_RenderRect(renderer, &keys[i].rect);
        }
    }
}
