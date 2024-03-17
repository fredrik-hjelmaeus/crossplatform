#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL2/SDL.h>
#include "opengl.h"
#include "types.h"

//struct Entity; // Forward declaration, replace with actual definition if needed

struct Globals {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    SDL_GLContext gl_context;
    int running;
    Entity* entities;
    int vertex_count;
    float delta_time;
    GLenum overideDrawMode;
    int overrideDrawModeBool;
};

extern struct Globals globals;

#endif // GLOBALS_H