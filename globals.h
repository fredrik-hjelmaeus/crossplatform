#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL2/SDL.h>

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
    bool mouseLeftButtonPressed;
    bool drawBoundingBoxes;
    Character characters[128];
    bool render;
    GpuData gpuFontData;
    float unitScale;
    Entity lights[1]; // TODO: temporary solution for lights?
    bool culling;
    int drawCallsCounter;
    bool debugDrawCalls;
};

extern struct Globals globals;
extern Camera uiCamera;
extern Camera mainCamera;

#endif 
