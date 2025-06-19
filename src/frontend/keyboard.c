#include <stdio.h>
#include <math.h>

#include "keyboard.h"


// Global flag to indicate if a note is currently playing (for monophonic backend)
static int note_is_playing = 0;
const float WHITE_KEY_WIDTH = 80.0f;
const float WHITE_KEY_HEIGHT = 240.0f;
const float BLACK_KEY_WIDTH = 48.0f;
const float BLACK_KEY_HEIGHT = 144.0f;

// Array to hold all keys (global for easy access in later functions)
PianoKey keys[NUM_WHITE_KEYS + NUM_BLACK_KEYS]; // One octave (C to C)

// Function to calculate frequency from MIDI note number  - Is this required??
double midi_to_frequency(int midi_note) {
    return 440.0 * pow(2.0, (double)(midi_note - 69) / 12.0);
}


int is_black(int offset_in_octave) {
        return (offset_in_octave == 1 || offset_in_octave == 3 ||
                offset_in_octave == 6 || offset_in_octave == 8 || offset_in_octave == 10);
}

// Initialize the keyboard - takes in top left coordinates for the keyboard
void init_keyboard(float start_x, float start_y) {
    int key_index = 0; //Tracks keys array for the key structs that are being initialised
    float current_x = start_x;

    // Define the pattern for white and black keys in one octave (C to C)
    // C, C#, D, D#, E, F, F#, G, G#, A, A#, B
    int white_key_midi_offsets[] = {0, 2, 4, 5, 7, 9, 11}; // C, D, E, F, G, A, B (octave)
    int black_key_midi_offsets[] = {1, 3, -1, 6, 8, 10}; // C#, D#, F#, G#, A# (-1 for gaps)

    // Helper to identify white/black key at a logical index (0=C, 1=C#, etc.)
    

    // Generate keys for one octave (C4 to B4/C5) for simplicity
    // C4 = MIDI 60, D4 = 62, E4 = 64, F4 = 65, G4 = 67, A4 = 69, B4 = 71
    // C#4 = 61, D#4 = 63, F#4 = 66, G#4 = 68, A#4 = 70

    // First, initialize white keys
    float white_key_pos_x = start_x;

    
    for (int i = 0; i < NUM_WHITE_KEYS; i++) { // C to C (8 keys)
        keys[key_index].midi_note = START_MIDI_NOTE_C4 + white_key_midi_offsets[i];
        keys[key_index].frequency = midi_to_frequency(keys[key_index].midi_note);
        keys[key_index].is_black_key = 0;
        keys[key_index].is_pressed = 0;
        keys[key_index].rect = (SDL_FRect){white_key_pos_x, start_y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT};
        white_key_pos_x += WHITE_KEY_WIDTH;
        key_index++;
    }

    // Then, initialize black keys (overlap white keys)
    // Positions are relative to the *white keys* they sit between
    // C# is between C and D
    // D# is between D and E
    // F# is between F and G
    // G# is between G and A
    // A# is between A and B
    key_index = NUM_WHITE_KEYS; // Start adding black keys after white keys
    float black_key_offset_x[] = {1.0f, 3.0f, 4.0f, 6.0f, 7.0f}; // Offsets from start of white key
    int black_key_midi_notes[] = {61, 63, 66, 68, 70}; // C#4, D#4, F#4, G#4, A#4

    for (int i = 0; i < NUM_BLACK_KEYS; i++) {
        keys[key_index].midi_note = black_key_midi_notes[i];
        keys[key_index].frequency = midi_to_frequency(keys[key_index].midi_note);
        keys[key_index].is_black_key = 1;
        keys[key_index].is_pressed = 0;
        keys[key_index].rect = (SDL_FRect){start_x + black_key_offset_x[i] * WHITE_KEY_WIDTH - BLACK_KEY_WIDTH / 2,
                                            start_y,
                                            BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT};
        key_index++;
    }
}

// Function to handle key press
void handle_key_press(int mouse_x, int mouse_y) {
    // Create an SDL_Point from mouse coordinates (casting to int implicitly)
    SDL_Point mouse_point = {mouse_x, mouse_y};

    // Check black keys first, as they overlap white keys
    for (int i = NUM_WHITE_KEYS; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; ++i) {
        // Need to convert SDL_FRect to SDL_Rect for SDL_PointInRect
        // This is a temporary conversion for the check, as SDL_PointInRect
        // in your current headers seems to only take SDL_Rect.
        SDL_Rect key_rect_int = {(int)keys[i].rect.x, (int)keys[i].rect.y,
                                 (int)keys[i].rect.w, (int)keys[i].rect.h};
        if (SDL_PointInRect(&mouse_point, &key_rect_int)) { // Use SDL_PointInRect
            if (!keys[i].is_pressed) {
                keys[i].is_pressed = 1;
                start_synth_note(keys[i].frequency, get_current_volume());
                note_is_playing = 1;
            }
            return;
        }
    }

    // Then check white keys
    for (int i = 0; i < NUM_WHITE_KEYS; ++i) {
        SDL_Rect key_rect_int = {(int)keys[i].rect.x, (int)keys[i].rect.y,
                                 (int)keys[i].rect.w, (int)keys[i].rect.h};
        if (SDL_PointInRect(&mouse_point, &key_rect_int)) { // Use SDL_PointInRect
            if (!keys[i].is_pressed) {
                keys[i].is_pressed = 1;
                start_synth_note(keys[i].frequency, get_current_volume());
                note_is_playing = 1;
            }
            return;
        }
    }
}

// Function to handle key release
void handle_key_release(int mouse_x, int mouse_y) {
    // Reset all pressed keys for a simpler monophonic release model
    // For polyphony, you'd track which key was pressed and release only that one.
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

    // Draw black keys second (so they appear on top)
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