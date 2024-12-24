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

#define MAX_LIGHTS 10

// Window dimensions
static const int width = 800;  // If these change, the views defaults should be changed aswell.
static const int height = 600; // If these change, the views defaults should be changed aswell.

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
    bool mouseDragged;
    bool drawBoundingBoxes;
    Character characters[128];
    int fontSize;
    Color textColor;
    bool render;
    GpuData gpuFontData;
    float unitScale;
    Entity lights[MAX_LIGHTS];
    int lightsCount;
    bool culling;
    int drawCallsCounter;
    bool debugDrawCalls;
    Arena assetArena;
    Arena uiArena;
    Material* materials;
    int materialsCount;
    int materialsCapacity;
    int objDataCapacity;
    int focusedEntityId;
    float charScale;
    bool mouseDoubleClick;
    bool blinnMode;
    bool gamma;
    GLuint depthMap;
    mat4x4 lightSpaceMatrix;
    GpuData depthMapBuffer; // used to store depthmap shader
    GpuData frameBuffer; // used to store framebuffer shader
    GpuData postProcessBuffer; // used to store framebuffer shader
    bool showDepthMap;

    // Cursor
    int cursorEntityId;
    float cursorBlinkTime;
    bool cursorSelectionActive;
    bool deselectCondition;
    float cursorDragStart;
    unsigned int cursorTextSelection[2];
};

extern struct Globals globals;
extern Camera uiCamera;
extern Camera mainCamera;

#define ASSET_MEMORY_SIZE 5000000
#define UI_MEMORY_SIZE 5000000

// Macro to allocate memory from the arena and cast it to the appropriate type
// Use this to get memory
//#define GET_MEMORY(ARENA,TYPE,COUNT) ((TYPE*)arena_Alloc(&ARENA,sizeof(TYPE) * (COUNT)))

#ifdef DEV_MODE
    #define ASSERT(Expression,message) if (!(Expression)) { fprintf(stderr, "\x1b[31mAssertion failed: %s\x1b[0m\n", message); *(int *)0 = 0; }
    #else  // Tell compiler to do nothing in release mode
    #define ASSERT(Expression, message) ((void)0)
#endif

#endif 
