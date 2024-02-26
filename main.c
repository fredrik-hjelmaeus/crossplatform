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

#include "opengl.h"
#include "utils.h"
#include <time.h>

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
float randFloat(float rmin, float rmax) {
    return (float)rand() / (float)RAND_MAX * (rmax - rmin) + rmin;
}



// Globals
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Event event;
SDL_GLContext gl_context = NULL;
int running = 1;
struct view {
    int x;
    int y;
    int width;
    int height;
};
struct view views[3] = {
    {0, 200, 800, 400},
    {0, 0, 800, 200},
    {0, 0, 800, 600}
};

    
// Window dimensions
static const int width = 800;
static const int height = 600;

void setRenderDrawColor(int r,int g, int b, int a) {
    // Initial renderer color
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

#ifdef __EMSCRIPTEN__
void setVersion() {
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES) != 0){
        printf("Failed to set OpenGL ES profile: %s\n", SDL_GetError());
    }
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) != 0){
        printf("Failed to set OpenGL ES major version: %s\n", SDL_GetError());
    }
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0) != 0){
        printf("Failed to set OpenGL ES minor version: %s\n", SDL_GetError());
    }
} 
#else
void setVersion() {
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) != 0){
        printf("Failed to set OpenGL core profile: %s\n", SDL_GetError());
    }
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) != 0){
        printf("Failed to set OpenGL major version: %s\n", SDL_GetError());
    }
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) != 0){
        printf("Failed to set OpenGL minor version: %s\n", SDL_GetError());
    }
}
#endif

void initWindow() {
    // Initialize SDL
    CHECK_ERROR(SDL_Init(SDL_INIT_VIDEO) != 0, SDL_GetError());

    // Set OpenGL version (3.0) or webgl won't be 2.0
    setVersion();

    // Create an SDL window
    window = SDL_CreateWindow(
        "Hello, SDL2", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        width, 
        height, 
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_RESIZABLE
    );
    CHECK_ERROR(window == NULL, SDL_GetError());

}

void setViewport(struct view view) {
    glViewport(view.x, view.y, view.width, view.height);
}

void initProgram(){
    initWindow();
}

int pollEvent(){
    return SDL_PollEvent(&event);
}

void input() {
    // Process events
    while(pollEvent()) {
        
        if(event.type == SDL_QUIT) {
            running = 0;
        } 
        if(event.type == SDL_KEYDOWN) {
            const char *key = SDL_GetKeyName(event.key.keysym.sym);
            if(strcmp(key, "C") == 0) {
                glClearColor(randFloat(0.0,1.0),randFloat(0.0,1.0),randFloat(0.0,1.0), 1.0);
            }
            if(strcmp(key, "Escape") == 0) {
                running = 0;
            }
            if(strcmp(key, "F") == 0) {
                Uint32 windowFlags = SDL_GetWindowFlags(window);
                if(windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                    SDL_SetWindowFullscreen(window, 0);
                } else {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
            }
        }
        if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
            int w, h; 
            SDL_GetWindowSize(window, &w, &h);
            printf("New Window size: %d x %d\n", w, h);
            // TODO: use this for reprojection later: float aspect = (float)w / (float)h;
            glViewport(0, 0, w / 2, h);
        }
    }
}



void update(){
    // Update game objects
}

void render(){

    // Clear the entire window
    glClear(GL_COLOR_BUFFER_BIT);

    // Render without ui on wasm
    #ifdef __EMSCRIPTEN__
    setViewport(views[2]);
    render_scene();
    #else
    // Render scene and ui on native
    setViewport(views[0]);
    render_scene();
    setViewport(views[1]);
    render_ui();
    #endif

    // Swap the window buffers to show the new frame
    SDL_GL_SwapWindow(window); 
}

void quit(){
    // Release resources
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void initOpenGLWindow(){
  // Create an OpenGL context associated with the window.
   gl_context = SDL_GL_CreateContext(window);
   if (!gl_context) {
      printf("Failed to create OpenGL context: %s\n", SDL_GetError());
      exit(1);
   }
   
   printf("OpenGL context created!\n");
}



#ifdef __EMSCRIPTEN__
void emscriptenLoop() {
    if(!running){
        printf("Closing sdl canvas!\n");
        quit();
        printf("Goodbye!\n");
        emscripten_cancel_main_loop();
    }
    input();
    update();
    render();
}
#endif

int main(int argc, char **argv) {
    // Initialize the random number generator
    srand((unsigned int)time(NULL));
    
    initProgram();
    initOpenGLWindow();
    setup_scene();
    setup_ui();
  
    // Wasm code
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscriptenLoop, 0, 1);
    #else
    // native code
    while(running) {
        input();
        update();
        render();
    }
    quit();
    printf("Non emscripten shutdown complete!\n");
    return 0;
    #endif
}
