#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL2/SDL.h>
//#include "opengl.h"
//#include "types.h"
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
    Views views;
    int firstMouse;
    float mouseXpos;
    float mouseYpos;
    bool drawBoundingBoxes;
    Character characters[128];
    bool render;
};

extern struct Globals globals;
extern Camera uiCamera;
extern Camera mainCamera;

#endif // GLOBALS_H
