#ifndef GLOBALS_H
#define GLOBALS_H

#include <SDL2/SDL.h>

#if defined(__linux__)
    #define PLATFORM "Linux"
#elif defined(_WIN32)
    #define PLATFORM "Windows"
#elif defined(__APPLE__)
    #define PLATFORM "macOS"
#elif defined(__EMSCRIPTEN__)
    #define PLATFORM "WebAssembly"
#else
    #define PLATFORM "Unknown"
#endif

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
    Arena assetArena;
    Material* materials;
    int materialsCount;
    int materialsCapacity;
    int objDataCapacity;
};

extern struct Globals globals;
extern Camera uiCamera;
extern Camera mainCamera;

#define ASSET_MEMORY_SIZE 5000000

// Macro to allocate memory from the arena and cast it to the appropriate type
// Use this to get memory
//#define GET_MEMORY(ARENA,TYPE,COUNT) ((TYPE*)arena_Alloc(&ARENA,sizeof(TYPE) * (COUNT)))

#ifdef DEV_MODE
    #define ASSERT(Expression,message) if (!(Expression)) { fprintf(stderr, "\x1b[31mAssertion failed: %s\x1b[0m\n", message); *(int *)0 = 0; }
    #else  // Tell compiler to do nothing in release mode
    #define ASSERT(Expression, message) ((void)0)
#endif

#endif 
