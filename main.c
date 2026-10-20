#include "input-state.h"
#include "render.h"
#include <stdio.h>
#include <time.h>
#include "opengl_types.h"
#include "types.h"
#include "app.h"
#include "linmath.h"
#include "utils.h"
#include "opengl.h"
#include "text.h"
#include "ecs.h"
#include "ecs-entity.h"
#include "ecs-systems.h"
#include "input-mapping.h"
#include "event.h"
#include "api.h"
#include "assets.h"
#include "ui.h"
#include "tests.h"
#include "shadows.h"
#include "fh-sdl.h"
#include "input.h"
#include "viewport.h"
#include "lighting.h"


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif


// Prototypes

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

// Colors
Color blue = {0.0f, 0.0f, 1.0f, 1.0f};
Color red = {1.0f, 0.0f, 0.0f, 1.0f};
Color green = {0.0f, 1.0f, 0.0f, 1.0f};

// Time variables
#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)
int last_frame_time = 0;

//------------------------------------------------------
// SDL, OpenGL & PROGRAM INITIALIZATION
//------------------------------------------------------


void initProgram(AppState *app){
    if (!initSdlWindow(app)) exit(1);
    initECS(app);
}

void displayFps(AppState *app, Uint32 ticks){
    app->frameCount++;
    
    if(ticks/1000-app->prevTick != 0){
        char str[10];
        sprintf(str,"FPS: %d", app->frameCount);
        setWindowTitle(app, str);
        app->frameCount = 0;
    }
    app->prevTick = ticks/1000;
}

void update(InputState *inputState, RenderState *renderState, AppState *app){

    // Time 
    Uint32 ticks = getTicks();

    // Cap the frame rate
    int time_to_wait = FRAME_TARGET_TIME - (ticks - last_frame_time);
    if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        delay((Uint32)time_to_wait);
    }

    // Set delta time in seconds
    app->delta_time = (getTicks() - last_frame_time) / 1000.0f;

    displayFps(app, ticks);
    
    // Systems
    cameraSystem(inputState, app);
    processEvents(inputState, app);
    uiPositionSystem(app);
    uiInputSystem(inputState, app);
    hoverAndClickSystem(inputState, app);
    uiSliderSystem(inputState, app);
    uiCheckboxSystem(renderState, app);
    textCursorSystem(inputState, app);
    movementSystem(app);
    modelSystem(app);
    inputState->prevMouseLeftDown = inputState->mouseLeftButtonPressed;
}

#ifdef __EMSCRIPTEN__
typedef struct {
    AppState *app;
    RenderState *renderState;
    InputState *inputState;
} LoopContext;

void emscriptenLoop(void *context) {
    LoopContext *loop = context;
    AppState *app = loop->app;
    RenderState *renderState = loop->renderState;
    InputState *inputState = loop->inputState;
    if(!app->running){
        printf("Closing sdl canvas!\n");
        quit(renderState, app);
        printf("Goodbye!\n");
        emscripten_cancel_main_loop();
        return;
    }
    input(inputState, renderState, app);
    update(inputState, renderState, app);
    render(renderState, app);
}
#endif



int main(int argc, char **argv) {
    
   /*  if(PLATFORM == "UNKNOWN") {
        printf("Platform is unknown, please set PLATFORM in the makefile\n");
        return 1;
    }
    printf("Platform is %s\n", PLATFORM); */

    AppState appState = appDefaults();
    RenderState renderState = renderDefaults();
    InputState inputState = inputDefaults();

    // Initialize Memory Arenas
    arena_initMemory(&appState.assetArena, ASSET_MEMORY_SIZE * sizeof(Vertex));
    arena_initMemory(&appState.uiArena, UI_MEMORY_SIZE * sizeof(char));
   
    // Sub memory allocations
    appState.materials = arena_Alloc(&appState.assetArena, appState.materialsCapacity * sizeof(Material));
    // BELOW NOT YET IMPLEMENTED
    //app->textures = arena_Alloc(&app->assetArena, app->texturesCapacity * sizeof(Texture));
    //app->meshes = arena_Alloc(&app->assetArena, app->meshCapacity * sizeof(MeshComponent));
    //app->groups/models = arena_Alloc(&app->assetArena, app->modelsCapacity * sizeof(GroupComponent));
    //app->cameras = arena_Alloc(&app->assetArena, app->camerasCapacity * sizeof(Camera));
    //app->ui_elements = arena_Alloc(&app->assetArena, app->uiElementsCapacity * sizeof(UIComponent));
        
    //obj_runTests();
    
    // Initialize the random number generator
    srand((unsigned int)time(NULL));
    
    initProgram(&appState);
   
    if (!createOpenGLContext(&renderState, &appState)) exit(1);
    glEnable(GL_DEPTH_TEST);

    // Tests
    /* tests_ui_coordinate_conversion();
    tests_binary_conversion();
    tests_events();
    return 0; */
    
    initScene(&renderState, &appState);
    
    //------------------------------------------------------
    // Main loop
    // -----------------------------------------------------
    // Wasm code
    #ifdef __EMSCRIPTEN__
    LoopContext loop = {&appState, &renderState, &inputState};
    emscripten_set_main_loop_arg(emscriptenLoop, &loop, 0, 1);
    #else
    // native code
    while(appState.running) {
        input(&inputState, &renderState, &appState);
        update(&inputState, &renderState, &appState);
        render(&renderState, &appState);
        debugSystem(&renderState);
    }

    // -----------------------------------------------------
    // Quit
    // -----------------------------------------------------
    quit(&renderState, &appState);
    printf("Non emscripten shutdown complete!\n");
    return 0;
    #endif
}




    
