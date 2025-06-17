#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


int main(void) {
    printf("Initializing SDL...\n");
    
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    printf("Creating window...\n");
    SDL_Window *window = SDL_CreateWindow(
        "Keyboard Test - CLICK HERE FIRST", 
        800, 600, 
        SDL_EVENT_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("No renderer, continuing anyway...\n");
    }
    
    printf("\n=== IMPORTANT ===\n");
    printf("1. Click on the SDL window to give it focus\n");
    printf("2. Then press keys to test\n");
    printf("3. Press ESC to exit\n");
    printf("=================\n\n");
    
    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    printf("Window closed\n");
                    running = false;
                    break;
                    
                case SDL_EVENT_KEY_DOWN:
                    printf("Key pressed: %s\n", SDL_GetKeyName(event.key.key));
                    if (event.key.key == SDLK_ESCAPE) {
                        running = false;
                    }
                    break;
                    
                case SDL_EVENT_WINDOW_FOCUS_GAINED:
                    printf("Window gained focus - ready for keyboard input!\n");
                    break;
                    
                case SDL_EVENT_WINDOW_FOCUS_LOST:
                    printf("Window lost focus\n");
                    break;
            }
        }
        
        if (renderer) {
            SDL_SetRenderDrawColor(renderer, 0, 100, 200, 255);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
        }
        
        SDL_Delay(16);
    }
    
    if (renderer) {
      SDL_DestroyRenderer(renderer);
      }
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    printf("Done!\n");
    return 0;
}