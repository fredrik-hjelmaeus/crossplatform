#ifndef APP_H
#define APP_H

#include <SDL2/SDL.h>
#include "types.h"
#include "scene.h"

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

typedef struct AppState {
    SDL_Window* window;
    SDL_Event event;
    int running;
    Entity* entities;
    int vertex_count;
    float delta_time;
    SceneMode currentSceneMode;
    Views views;
    int firstMouse;
    float mouseXpos;
    float mouseYpos;
    bool mouseLeftButtonPressed;
    bool prevMouseLeftDown;
    bool mouseDragged;
    vec2 mouseDragStart;
    vec2 mouseDragPreviousFrame;
    Character characters[128];
    int fontSize;
    Color textColor;
    float unitScale;
    // For now, scene lights stay here: entity creation and UI editing use them too.
    Light lights[MAX_LIGHTS];
    int lightsCount;
    Arena assetArena;
    Arena uiArena;
    Arena eventArena;
    // For now, asset materials stay here: loading and entity creation own this catalog.
    Material* materials;
    int materialsCount;
    int materialsCapacity;
    int objDataCapacity;
    int focusedEntityId;
    float charScale;
    bool mouseDoubleClick;

    // Cursor
    int cursorEntityId;
    float cursorBlinkTime;
    bool cursorSelectionActive;
    bool deselectCondition;
    float cursorDragStart;
    unsigned int cursorTextSelection[2];

    int frameCount;
    Uint32 prevTick;
    bool showUI;

} AppState;

AppState appDefaults(void);



#define ASSET_MEMORY_SIZE 5000000
#define UI_MEMORY_SIZE 5000000

#ifdef DEV_MODE
 /*    #define ASSERT(Expression,message) if (!(Expression)) { fprintf(stderr, "\x1b[31mAssertion failed: %s\x1b[0m\n", message); *(int *)0 = 0; }
    #else  // Tell compiler to do nothing in release mode
     */
    #define ASSERT(Expression, message) \
    if (!(Expression)) { \
        fprintf(stderr, "\x1b[31mAssertion failed: %s\x1b[0m\n", message); \
        abort(); \
    }
#else
    #define ASSERT(Expression, message) ((void)0)
#endif 

#endif // APP_H end
