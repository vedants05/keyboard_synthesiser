#include <SDL3/SDL.h>
#include <stdio.h> // For fprintf and stderr
#include <assert.h>

int init_SDL(void){
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    return 0;
}

SDL_Window* init_window(void){
    SDL_Window* window = SDL_CreateWindow(
        "Synth34",
        800,
        600,
        SDL_WINDOW_RESIZABLE
    );
    if (window == NULL) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return NULL;
    }
    SDL_MaximizeWindow(window); //makes it full screen on startup
    return window;
}

SDL_Renderer *init_renderer(SDL_Window *window){
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        NULL// Use the first available rendering driver  
    );

    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return NULL;
    }
    return renderer;
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


void run(SDL_Renderer *renderer){
    SDL_Event e;
    int quit = 0;

    while (!quit) {
        // Process events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_EVENT_QUIT) {
                quit = 1; // User requested to quit
            }
            // You can add more event handling here, e.g., keyboard presses, mouse clicks
        }

        // Drawing operations (example: clear with a blue background)
        SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xFF); // Blue color (RGBA)
        SDL_RenderClear(renderer);

        // Update the screen with the rendering
        SDL_RenderPresent(renderer);
    }

}

int main(int argc, char* argv[]) {
    init_SDL();
    SDL_Window *window = init_window();
    assert(window != NULL);
    SDL_Renderer *renderer = init_renderer(window);
    assert(renderer != NULL);
    run(renderer);
    cleanup(window, renderer);

    return 0;
}