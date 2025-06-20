#include <stdio.h>
#include <math.h>

#include "keyboard.h"


// Global flag to indicate if a note is currently playing (for monophonic backend)
static int note_is_playing = 0;
// Global octave offset (can be changed with [ and ] keys)
static int octave_offset = 0;
const float WHITE_KEY_WIDTH = 80.0f;
const float WHITE_KEY_HEIGHT = 240.0f;
const float BLACK_KEY_WIDTH = 48.0f;
const float BLACK_KEY_HEIGHT = 144.0f;

// Array to hold all keys (global for easy access in later functions)
PianoKey keys[NUM_WHITE_KEYS + NUM_BLACK_KEYS]; // One octave (C to C)

// Map physical keys to virtual piano keys
const SDL_Scancode white_key_scancodes[NUM_WHITE_KEYS] = {
    SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F, SDL_SCANCODE_G,
    SDL_SCANCODE_H, SDL_SCANCODE_J, SDL_SCANCODE_K, SDL_SCANCODE_L
};

const SDL_Scancode black_key_scancodes[NUM_BLACK_KEYS] = {
    SDL_SCANCODE_E, SDL_SCANCODE_T, SDL_SCANCODE_Y, SDL_SCANCODE_I, SDL_SCANCODE_O
};

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

    // C, C#, D, D#, E, F, F#, G, G#, A, A#, B
    int white_key_midi_offsets[] = {0, 2, 4, 5, 7, 9, 11}; // C, D, E, F, G, A, B (octave)
    int black_key_midi_offsets[] = {1, 3, -1, 6, 8, 10}; // C#, D#, F#, G#, A# (-1 for gaps)
    
    // A3 = MIDI 57, B3 = 59, C4 = 60, D4 = 62, E4 = 64, F4 = 65, G4 = 67
    // A#3 = 58, C#4 = 61, D#4 = 63, F#4 = 66, G#4 = 68

    float white_key_pos_x = start_x;
    int white_key_midi_notes[] = {57, 59, 60, 62, 64, 65, 67, 69}; // A3, B3, C4, D4, E4, F4, G4, A4
    
    for (int i = 0; i < NUM_WHITE_KEYS; i++) { 
        keys[key_index].midi_note = white_key_midi_notes[i];
        keys[key_index].frequency = midi_to_frequency(keys[key_index].midi_note);
        keys[key_index].is_black_key = 0;
        keys[key_index].is_pressed = 0;
        keys[key_index].rect = (SDL_FRect){white_key_pos_x, start_y, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT};
        white_key_pos_x += WHITE_KEY_WIDTH;
        keys[key_index].scancode = white_key_scancodes[i];
        key_index++;
    }


    key_index = NUM_WHITE_KEYS; // Start adding black keys after white keys
    float black_key_offset_x[] = {1.0f, 3.0f, 4.0f, 6.0f, 7.0f}; // Offsets from start of white key
    int black_key_midi_notes[] = {58, 61, 63, 66, 68}; // A#3, C#4, D#4, F#4, G#4

    for (int i = 0; i < NUM_BLACK_KEYS; i++) {
        keys[key_index].midi_note = black_key_midi_notes[i];
        keys[key_index].frequency = midi_to_frequency(keys[key_index].midi_note);
        keys[key_index].is_black_key = 1;
        keys[key_index].is_pressed = 0;
        keys[key_index].rect = (SDL_FRect){start_x + black_key_offset_x[i] * WHITE_KEY_WIDTH - BLACK_KEY_WIDTH / 2,
                                            start_y,
                                            BLACK_KEY_WIDTH, BLACK_KEY_HEIGHT};
        keys[key_index].scancode = black_key_scancodes[i];
        key_index++;
    }
}

void handle_physical_key_down(SDL_Scancode scancode) {
    // If a note is already playing, stop it first (monophonic behavior)
    if (note_is_playing) {
        stop_synth_note();
        note_is_playing = 0;
        // Also un-press any key that was held down
        for (int i = 0; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; i++) {
            keys[i].is_pressed = 0;
        }
    }

    // Find the key that corresponds to the pressed scancode
    for (int i = 0; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; i++) {
        if (keys[i].scancode == scancode) {
            if (!keys[i].is_pressed) {
                keys[i].is_pressed = 1;
                // Apply octave offset to the frequency
                int adjusted_midi = keys[i].midi_note + (octave_offset * 12);
                // Clamp to valid MIDI range (0-127)
                if (adjusted_midi < 0) adjusted_midi = 0;
                if (adjusted_midi > 127) adjusted_midi = 127;
                double adjusted_frequency = midi_to_frequency(adjusted_midi);
                start_synth_note(adjusted_frequency, get_current_volume());
                note_is_playing = 1;
            }
            return; // Found our key, no need to check others
        }
    }
}

// --- NEW: Handler for physical key up event ---
void handle_physical_key_up(SDL_Scancode scancode) {
    // Find the key that corresponds to the released scancode
    for (int i = 0; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; i++) {
        if (keys[i].scancode == scancode) {
            // Only stop the note if this is the key that was playing
            if (keys[i].is_pressed) {
                keys[i].is_pressed = 0;
                if (note_is_playing) {
                    stop_synth_note();
                    note_is_playing = 0;
                }
            }
            return; // Found our key
        }
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
