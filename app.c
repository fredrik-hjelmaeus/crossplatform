#include "app.h"
#include "render.h"
#include "utils.h"
#include "tests.h"
#include <time.h>
#include "input-state.h"
#include "ecs-systems.h"
#include "event.h"
#include "fh-sdl.h"
#include <stdio.h>
#include "ecs.h"

//------------------------------------------------------
// App variables / Initialization
//------------------------------------------------------
AppState appDefaults(void) {
    return (AppState){
    
    .window=NULL,
    .running=1,
    .entities=NULL,
    .vertex_count=0,
    .delta_time=0.0f,
    .last_frame_time=0,
    .views = {
        .ui={{0,   0, width, height}, {0.0f, 1.0f, 0.0f, 1.0f},NULL,false},
        .main={{0, 0, width, height}, {1.0f, 0.0f, 0.0f, 1.0f},NULL,false},
        .full={{0, 0, width, height}, {0.0f, 0.0f, 0.0f, 1.0f} ,NULL,false},
    },
    .currentSceneMode=SCENE_MODE_NONE,

    .deselectCondition=false,

    .charScale=0.5f,
    .fontSize=26,
    .textColor={187.0/255.0,188.0/255.0,196.0/255.0,1.0},
    .unitScale=100.0f, // 1 unit = 1 cm 
    //.assetArena=NULL,
    .materials=NULL,
    .materialsCount=0,
    .materialsCapacity=15,
    .objDataCapacity=20000,
    .lights={{0}},
    .lightsCount=0,
    .focusedEntityId=-1,

    // Cursor
    .cursorEntityId=-1,
    .cursorBlinkTime=0.5f,
    .cursorSelectionActive=false,
    .cursorDragStart=-1.0f,
    .cursorTextSelection={0,0},
    
    .frameCount = 0,
    .prevTick = 0,
    .showUI = true,
    };
}

// Time variables
#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

static void displayFps(AppState *app, Uint32 ticks){
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
    int time_to_wait = FRAME_TARGET_TIME - (ticks - app->last_frame_time);
    if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        delay((Uint32)time_to_wait);
    }

    // Set delta time in seconds
    app->delta_time = (getTicks() - app->last_frame_time) / 1000.0f;

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


void initProgram(AppState *app){
    if (!initSdlWindow(app)) exit(1);
    initECS(app);
}

void appInit(AppState *app, RenderState *renderState){
    // Initialize Memory Arenas
    arena_initMemory(&app->assetArena, ASSET_MEMORY_SIZE * sizeof(Vertex));
    arena_initMemory(&app->uiArena, UI_MEMORY_SIZE * sizeof(char));
   
    // Sub memory allocations
    app->materials = arena_Alloc(&app->assetArena, app->materialsCapacity * sizeof(Material));
    // BELOW NOT YET IMPLEMENTED
    //app->textures = arena_Alloc(&app->assetArena, app->texturesCapacity * sizeof(Texture));
    //app->meshes = arena_Alloc(&app->assetArena, app->meshCapacity * sizeof(MeshComponent));
    //app->groups/models = arena_Alloc(&app->assetArena, app->modelsCapacity * sizeof(GroupComponent));
    //app->cameras = arena_Alloc(&app->assetArena, app->camerasCapacity * sizeof(Camera));
    //app->ui_elements = arena_Alloc(&app->assetArena, app->uiElementsCapacity * sizeof(UIComponent));
        
    //obj_runTests();
    
    // Initialize the random number generator
    srand((unsigned int)time(NULL));
    
    initProgram(app);
   
    if (!createOpenGLContext(renderState, app)) exit(1);
    glEnable(GL_DEPTH_TEST);

    // Tests
    /* tests_ui_coordinate_conversion();
    tests_binary_conversion();
    tests_events();
    return 0; */
    
    initScene(renderState, app);
    
}
