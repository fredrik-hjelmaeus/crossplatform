#include <stdio.h>
// macOS:
// build setup with .sh-file
// renderers:
// quartz : https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/drawingwithquartz2d/Introduction/Introduction.html
// SFML: https://github.com/SFML/CSFML
// gcc -o my_program newfile.c -I./include -L./lib -lsfml-graphics -lsfml-window -lsfml-system
// Vulkan
// Allegro
// Cairo
// opengl
// metal
// scenekit
// Core Animation
// Core Image
// SpriteKit
// imGui
// GTK
// sdl
#include <SDL2/SDL.h>
// Qt
// Raylib

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "utils.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Utility macros
#define CHECK_ERROR(test, message) \
    do { \
        if((test)) { \
            fprintf(stderr, "%s\n", (message)); \
            exit(1); \
        } \
    } while(0)

// Get a random number from 0 to 255
int randInt(int rmin, int rmax) {
    return rand() % rmax + rmin;
}

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Event event;
int running = 1;
    
// Window dimensions
static const int width = 800;
static const int height = 600;

void initWindow() {
    // Initialize SDL
    CHECK_ERROR(SDL_Init(SDL_INIT_VIDEO) != 0, SDL_GetError());
    // Create an SDL window
    window = SDL_CreateWindow("Hello, SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    CHECK_ERROR(window == NULL, SDL_GetError());
}

void initRenderer() {
    // Create a renderer (accelerated and in sync with the display refresh rate)
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);    
    CHECK_ERROR(renderer == NULL, SDL_GetError());
}

void setRenderDrawColor(int r,int g, int b, int a) {
    // Initial renderer color
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

int pollEvent(){
    return SDL_PollEvent(&event);
}

void processInput() {
    // Process events
    while(pollEvent()) {
        if(event.type == SDL_QUIT) {
            running = 0;
        } else if(event.type == SDL_KEYDOWN) {
            const char *key = SDL_GetKeyName(event.key.keysym.sym);
            if(strcmp(key, "C") == 0) {
                SDL_SetRenderDrawColor(renderer, randInt(0, 255), randInt(0, 255), randInt(0, 255), 255);
            }
            if(strcmp(key, "Escape") == 0) {
                running = 0;
            }
        }
    }
}

void update(){
    // Update game objects
}

void clearRenderScreen(){
    SDL_RenderClear(renderer);
}

void render(){
    // Show what was drawn
    SDL_RenderPresent(renderer);
}

void quit(){
    // Release resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
#ifdef __EMSCRIPTEN__
void emscriptenLoop() {
    if(!running){
        printf("Closing sdl canvas!\n");
        quit();
        printf("Goodbye!\n");
        emscripten_cancel_main_loop();
    }
    processInput();
    update();
    clearRenderScreen();
    render();
}
#endif

int main(int argc, char **argv) {
    // Initialize the random number generator
    srand((unsigned int)time(NULL));
    print_hello();
    
    initWindow();
    initRenderer();
    setRenderDrawColor(255, 255, 255, 255);

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscriptenLoop, 0, 1);
    #else
    while(running) {
        processInput();
        update();
        clearRenderScreen();
        render();
    }
    quit();
    printf("Non emscripten shutdown complete!\n");
    return 0;
    #endif
}
