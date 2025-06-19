#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "utils.h"

#define NUM_WHITE_KEYS 8
#define NUM_BLACK_KEYS 5
#define START_MIDI_NOTE_C4 60 // MIDI note number for Middle C (C4)

// Key dimensions
extern const float WHITE_KEY_WIDTH;
extern const float WHITE_KEY_HEIGHT;
extern const float BLACK_KEY_WIDTH;
extern const float BLACK_KEY_HEIGHT; // Black keys are shorter

// Key structure
typedef struct {
    SDL_FRect rect;
    int midi_note; //Note number to identify a key
    double frequency;
    int is_black_key;
    int is_pressed;
    SDL_Scancode scancode;
} PianoKey;


double midi_to_frequency(int midi_note);
int is_black(int offset_in_octave);
void init_keyboard(float start_x, float start_y);
void handle_key_press(int mouse_x, int mouse_y);
void handle_key_release(int mouse_x, int mouse_y);
void draw_keyboard(SDL_Renderer *renderer);

//Key presses on computer keyboard
void handle_physical_key_up(SDL_Scancode scancode);
void handle_physical_key_down(SDL_Scancode scancode);

#endif