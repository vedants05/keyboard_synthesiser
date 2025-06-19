#include <stdio.h>
#include <SDL3/SDL_main.h>
#include "frontend/button.h"
#include "frontend/slider.h"
#include "frontend/keyboard.h"
#include "run.h"
#include "frontend/utils.h"

int init_SDL(void){
    // Initialize SDL (Video only, as audio is backend)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "could not initialize SDL3: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
    SDL_Log("TTF_Init failed: %s", SDL_GetError());
    return 1;
    }
    return 0;
}
int init_window(SDL_Window **window){
    // --- Window and Renderer Setup ---
    *window = SDL_CreateWindow(
        "Synth34",
        900, // Width to accommodate keyboard
        600, // Height
        SDL_WINDOW_RESIZABLE
    );
    if (window == NULL) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    return 0;
}

int init_renderer(SDL_Window *window, SDL_Renderer **renderer){
    *renderer = SDL_CreateRenderer(window, NULL);
    if (*renderer == NULL) {
        fprintf(stderr, "could not create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    return 0;
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void add_sliders(Slider *freq_slider, Slider *vol_slider){
    // Frequency Slider (20Hz to 20000Hz, for finer control from keys)
    init_slider(freq_slider, 750.0f, 100.0f, 20.0f, 300.0f,
                20.0f, 20000.0f, get_current_frequency(), "Frequency",
                (void (*)(float))set_synth_frequency);

    // Volume Slider (0.0 to 1.0)
    init_slider(vol_slider, 800.0f, 100.0f, 20.0f, 300.0f,
                0.0f, 1.0f, get_current_volume(), "Volume",
                (void (*)(float))set_synth_volume);

}

void add_buttons(Button *sine_button, Button *square_button, Button *saw_button) {
    Button sine = { {50.0f, 50.0f, 100.0f, 40.0f}, "Sine", 0, NULL };
    Button square = { {160.0f, 50.0f, 100.0f, 40.0f}, "Square", 0, NULL };
    Button saw = { {270.0f, 50.0f, 100.0f, 40.0f}, "Saw", 0, NULL };
    *sine_button = sine;
    *square_button = square;
    *saw_button = saw;
}

void add_GUI_elements(Slider *freq_slider, Slider *vol_slider,Button *sine_button, Button *square_button, Button *saw_button){
    // Adding Sliders 
    add_sliders(freq_slider, vol_slider);

    
    // Adding Waveform Buttons
    add_buttons(sine_button, square_button, saw_button);

    // Set initial waveform button state
    if (get_current_waveform_type() == 0) {
        sine_button->is_selected = 1;
        square_button->is_selected = 0;
        saw_button->is_selected = 0;
    } else if (get_current_waveform_type() == 1) {
        sine_button->is_selected = 0;
        square_button->is_selected = 1;
        saw_button->is_selected = 0;
    } else {
        sine_button->is_selected = 0;
        square_button->is_selected = 0;
        saw_button->is_selected = 1;
    }

    // Initialize the keyboard (placed below other controls)
    init_keyboard(50.0f, 300.0f); // X, Y position for keyboard start
}

void run(SDL_Renderer *renderer){
    Slider freq_slider;
    Slider vol_slider;
    Button sine_button;
    Button square_button;
    Button saw_button;
    TTF_Font *font = TTF_OpenFont("../src/frontend/fonts/arial.ttf", 20);

    add_GUI_elements(&freq_slider, &vol_slider, &sine_button, &square_button, &saw_button);

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
        draw_slider(renderer, font, &freq_slider);
        draw_slider(renderer, font, &vol_slider);
        draw_button(renderer,font, &sine_button);
        draw_button(renderer,font, &square_button);
        draw_button(renderer,font, &saw_button);

        // Draw the keyboard
        draw_keyboard(renderer);

        // Update the screen
        SDL_RenderPresent(renderer);
    }
}

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if (init_SDL() != 0) return 1;
    if (init_window(&window) != 0) return 1;
    if (init_renderer(window, &renderer) != 0) return 1;
    run(renderer);
    cleanup(window, renderer);

    return 0;
}