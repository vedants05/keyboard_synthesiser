#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
//#include <SDL3_ttf/SDL_ttf.h> 
#include <stdio.h>
#include <math.h> // For pow() to calculate frequencies

// --- Backend Interface (REPLACE THESE WITH YOUR ACTUAL BACKEND CALLS) ---
// These are placeholder functions. Your actual backend would implement these
// and do the real audio synthesis work based on these parameters.

static double current_backend_frequency = 440.0;
static double current_backend_volume = 0.5;
static int current_backend_waveform_type = 0; // 0: Sine, 1: Square

// Global flag to indicate if a note is currently playing (for monophonic backend)
static int note_is_playing = 0;

void set_synth_frequency(double freq) {
    if (freq < 20.0) freq = 20.0; // Min frequency
    if (freq > 20000.0) freq = 20000.0; // Max frequency (increased range for keyboard)
    current_backend_frequency = freq;
    // In your real backend: Send 'freq' to your audio generation logic.
    // If your backend handles note-on/off, you'd send a note-on here for this frequency
    // printf("Backend: Setting frequency to %.2f Hz\n", freq);
}

void set_synth_volume(double vol) {
    if (vol < 0.0) vol = 0.0;
    if (vol > 1.0) vol = 1.0;
    current_backend_volume = vol;
    // In your real backend: Send 'vol' to your audio generation logic.
    // printf("Backend: Setting volume to %.2f\n", vol);
}

void set_synth_waveform(int waveform_type) {
    current_backend_waveform_type = waveform_type;
    // In your real backend: Tell your audio generation logic to switch waveform.
    // printf("Backend: Setting waveform to %s\n", (waveform_type == 0) ? "Sine" : "Square");
}

// Function to tell the backend to stop playing sound (for monophonic playback)
void stop_synth_note() {
    // In your real backend: Tell your audio generation logic to stop current note.
    // This might mean setting amplitude to 0 or sending a note-off event.
    // For our simple backend, we'll set volume to 0.
    set_synth_volume(0.0);
    // printf("Backend: Stopping note.\n");
}

// Function to start a note at a given frequency
void start_synth_note(double freq, double vol) {
    set_synth_frequency(freq);
    set_synth_volume(vol); // Set volume to actual desired volume
    // printf("Backend: Starting note at %.2f Hz with volume %.2f\n", freq, vol);
}


double get_current_frequency() {
    return current_backend_frequency;
}

double get_current_volume() {
    return current_backend_volume;
}

int get_current_waveform_type() {
    return current_backend_waveform_type;
}

// --- Frontend GUI Logic ---

//------------------------------------------------------------------SLIDER STRUCTURE---------------------------------------------------------------------

// Structure for a generic slider
typedef struct {
    SDL_FRect track_rect; //Size of sliders background track
    SDL_FRect handle_rect; //Size of handle for slider
    float value; // Stores sliders current value as a ratio between 0.0 (bottom) and 1.0 (top)
    float min_val; //Actual min and maximum values the slider represents
    float max_val;
    int is_dragging; //Boolean value to track whether the user is currently holding the mouse down
    void (*on_value_changed)(float); // Function to be called when value of slider changes - takes in a float which represents the current value of slider
    char *label; //Label to be written on slider
} Slider;

// Initializing a slider
void init_slider(Slider *slider, float x, float y, float width, float height,
                 float min_val, float max_val, float initial_val, char *label,
                 void (*callback)(float)) {


    slider->track_rect = (SDL_FRect){x, y, width, height};

    // Calculate initial handle position based on initial_val relative to min/max - transforms intital_val into its position on slider between 0 and 1
    float initial_norm_pos = (initial_val - min_val) / (max_val - min_val);
    slider->handle_rect = (SDL_FRect){x - width / 2, y + height * (1.0f - initial_norm_pos) - 10, width * 2, 20.0f}; //Calculating handle position relative to slider track

    slider->value = initial_norm_pos; // Normalized 0.0-1.0
    slider->min_val = min_val;
    slider->max_val = max_val;
    slider->is_dragging = 0;
    slider->on_value_changed = callback;
    slider->label = label;

    // Safety check to ensure that the calculated starting position of the handle is never visually outside the bounds of the track
    if (slider->handle_rect.y < slider->track_rect.y) {
        slider->handle_rect.y = slider->track_rect.y;
    } 
    if (slider->handle_rect.y > slider->track_rect.y + slider->track_rect.h - slider->handle_rect.h) {
        slider->handle_rect.y = slider->track_rect.y + slider->track_rect.h - slider->handle_rect.h;
    }

}

// Update slider value based on mouse Y position if user is dragging the slider
void update_slider_from_mouse(Slider *slider, int mouse_y) {

    //Calulating minimum and maximum possible y coordinates for the top edge of the handle so it can't move off the track
    float handle_y_min = slider->track_rect.y;
    float handle_y_max = slider->track_rect.y + slider->track_rect.h - slider->handle_rect.h;

    //Sets the handle's new vertical position to be centered on the mouse cursor
    slider->handle_rect.y = (float)mouse_y - slider->handle_rect.h / 2.0f;

    // Fixing handle position within slider bounds
    if (slider->handle_rect.y < handle_y_min) {
        slider->handle_rect.y = handle_y_min;
    } else if (slider->handle_rect.y > handle_y_max) {
        slider->handle_rect.y = handle_y_max;
    }

    // Convert handles current y position to the value relative to slider value between 0 and 1
    float normalized_pos = 1.0f - ((slider->handle_rect.y - handle_y_min) / (handle_y_max - handle_y_min));
    slider->value = normalized_pos; // Normalized 0.0-1.0
    //Actual value relative to actual numbers for the slider
    float actual_value = slider->min_val + normalized_pos * (slider->max_val - slider->min_val);

    // If a callback function was assigned to the slider, pass in the updated value
    if (slider->on_value_changed) {
        slider->on_value_changed(actual_value);
    }
}

// Render a slider
void draw_slider(SDL_Renderer *renderer, Slider *slider) {
    // Draw track (grey)
    SDL_SetRenderDrawColor(renderer, 0x88, 0x88, 0x88, 0xFF);
    SDL_RenderFillRect(renderer, &slider->track_rect);

    // Draw handle (white)
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &slider->handle_rect);

    // TODO: Add text label using SDL_ttf
}

// ---------------------------------------------------------------BUTTON STRUCTURE-----------------------------------------------------------------------------------------------------
typedef struct {
    SDL_FRect rect; //SDL2 data type that defines a rectangle (stores x,y for height and width) with floating point precision
    char *label; //Stores text to be displayed on button
    int is_selected; //Boolean variable tracking state of button
    void (*on_click)(void); //Function pointer that requires takes in a function that has void input/outputs and this function is executed when the button is clicked
} Button;


// Render a button - renderer object is responsible for drawing all items in the window
void draw_button(SDL_Renderer *renderer, Button *button) {
    if (button->is_selected) {
        SDL_SetRenderDrawColor(renderer, 0x00, 0xAA, 0x00, 0xFF); // Green if selected - The four hex arguments are red, green, blue and alpha
    } else {
        SDL_SetRenderDrawColor(renderer, 0x44, 0x44, 0x44, 0xFF); // Dark grey if not
    }
    SDL_RenderFillRect(renderer, &button->rect); //Draws a solid, filled rectangle from the colour set using the if/else block above 
                                                 //Takes in a pointer to the rect of button which stores height and width of button

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); // White border
    SDL_RenderRect(renderer, &button->rect); // Draws the outline of the rectangle with the colour border selected above

    // TODO: Add text label using SDL_ttf
}

// --------------------------------------------------------------- KEYBOARD STRUCTURE ------------------------------------------------------------------------------------------------
#define NUM_WHITE_KEYS 7
#define NUM_BLACK_KEYS 5
#define START_MIDI_NOTE_C4 60 // MIDI note number for Middle C (C4)

// Key dimensions
const float WHITE_KEY_WIDTH = 80.0f;
const float WHITE_KEY_HEIGHT = 240.0f;
const float BLACK_KEY_WIDTH = 48.0f;
const float BLACK_KEY_HEIGHT = 144.0f; // Black keys are shorter

// Key structure
typedef struct {
    SDL_FRect rect;
    int midi_note; //Note number to identify a key
    double frequency;
    int is_black_key;
    int is_pressed;
} PianoKey;

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

    
    for (int i = 0; i < NUM_WHITE_KEYS; ++i) { // C to C (8 keys)
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
    float black_key_offset_x[] = {0.65f, 1.65f, 3.65f, 4.65f, 5.65f}; // Offsets from start of white key
    int black_key_midi_notes[] = {61, 63, 66, 68, 70}; // C#4, D#4, F#4, G#4, A#4

    for (int i = 0; i < NUM_BLACK_KEYS; ++i) {
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
    for (int i = 0; i < NUM_WHITE_KEYS + NUM_BLACK_KEYS; ++i) {
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

//------------------------------------------------------------------------MAIN FUNCTION---------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    // Initialize SDL (Video only, as audio is backend)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "could not initialize SDL3: %s\n", SDL_GetError());
        return 1;
    }

    // --- Window and Renderer Setup ---
    window = SDL_CreateWindow(
        "SDL3 Synthesiser Frontend",
        900, // Width to accommodate keyboard
        600, // Height
        SDL_WINDOW_RESIZABLE
    );
    if (window == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) {
        fprintf(stderr, "could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // --- GUI Elements Initialization ---

    // Frequency Slider (20Hz to 20000Hz, for finer control from keys)
    Slider freq_slider;
    init_slider(&freq_slider, 750.0f, 100.0f, 20.0f, 300.0f,
                20.0f, 20000.0f, get_current_frequency(), "Frequency",
                (void (*)(float))set_synth_frequency);

    // Volume Slider (0.0 to 1.0)
    Slider vol_slider;
    init_slider(&vol_slider, 800.0f, 100.0f, 20.0f, 300.0f,
                0.0f, 1.0f, get_current_volume(), "Volume",
                (void (*)(float))set_synth_volume);

    // Waveform Buttons
    Button sine_button = { {50.0f, 50.0f, 100.0f, 40.0f}, "Sine", 0, NULL };
    Button square_button = { {160.0f, 50.0f, 100.0f, 40.0f}, "Square", 0, NULL };

    // Set initial waveform button state
    if (get_current_waveform_type() == 0) {
        sine_button.is_selected = 1;
        square_button.is_selected = 0;
    } else {
        sine_button.is_selected = 0;
        square_button.is_selected = 1;
    }

    // Initialize the keyboard (placed below other controls)
    init_keyboard(50.0f, 300.0f); // X, Y position for keyboard start

    // --- Main Event Loop ---
    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = 1;
            } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                // Check slider interactions
                // Check slider interactions
                // ... inside SDL_EVENT_MOUSE_BUTTON_DOWN ...
                if (e.button.button == SDL_BUTTON_LEFT) {
                    // Create an SDL_Point from mouse coordinates
                    SDL_Point mouse_point = {(int)e.button.x, (int)e.button.y};

                    // For sliders and buttons, convert their FRect to Rect for the check
                    SDL_Rect freq_handle_rect_int = {(int)freq_slider.handle_rect.x, (int)freq_slider.handle_rect.y,
                                                    (int)freq_slider.handle_rect.w, (int)freq_slider.handle_rect.h};
                    if (SDL_PointInRect(&mouse_point, &freq_handle_rect_int)) {
                        freq_slider.is_dragging = 1;
                    }

                    SDL_Rect vol_handle_rect_int = {(int)vol_slider.handle_rect.x, (int)vol_slider.handle_rect.y,
                                                    (int)vol_slider.handle_rect.w, (int)vol_slider.handle_rect.h};
                    if (SDL_PointInRect(&mouse_point, &vol_handle_rect_int)) {
                        vol_slider.is_dragging = 1;
                    }

                    SDL_Rect sine_button_rect_int = {(int)sine_button.rect.x, (int)sine_button.rect.y,
                                                    (int)sine_button.rect.w, (int)sine_button.rect.h};
                    if (SDL_PointInRect(&mouse_point, &sine_button_rect_int)) {
                        set_synth_waveform(0);
                        sine_button.is_selected = 1;
                        square_button.is_selected = 0;
                    }

                    SDL_Rect square_button_rect_int = {(int)square_button.rect.x, (int)square_button.rect.y,
                                                    (int)square_button.rect.w, (int)square_button.rect.h};
                    if (SDL_PointInRect(&mouse_point, &square_button_rect_int)) {
                        set_synth_waveform(1);
                        sine_button.is_selected = 0;
                        square_button.is_selected = 1;
                    }

                    // Handle keyboard presses (if not dragging a slider)
                    if (!freq_slider.is_dragging && !vol_slider.is_dragging) {
                        handle_key_press(e.button.x, e.button.y); // mouse_x, mouse_y are ints already
                    }
                }

            } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    // Release any dragging sliders
                    freq_slider.is_dragging = 0;
                    vol_slider.is_dragging = 0;
                    // Handle keyboard release
                    handle_key_release(e.button.x, e.button.y);
                }
            } else if (e.type == SDL_EVENT_MOUSE_MOTION) {
                if (freq_slider.is_dragging) {
                    update_slider_from_mouse(&freq_slider, e.motion.y);
                }
                if (vol_slider.is_dragging) {
                    update_slider_from_mouse(&vol_slider, e.motion.y);
                } else {
                    // This logic is for highlighting keys on hover, if you wanted that.
                    // For now, it's implicitly handled by `handle_key_press` on click.
                }
            }
        }

        // --- Rendering ---
        SDL_SetRenderDrawColor(renderer, 0x22, 0x22, 0x22, 0xFF); // Darker grey background
        SDL_RenderClear(renderer);

        // Draw GUI elements
        draw_slider(renderer, &freq_slider);
        draw_slider(renderer, &vol_slider);
        draw_button(renderer, &sine_button);
        draw_button(renderer, &square_button);

        // Draw the keyboard
        draw_keyboard(renderer);

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    // --- Cleanup ---
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}