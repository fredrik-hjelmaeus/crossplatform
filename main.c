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

// STB
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif


// Prototypes
Camera* initCamera(void);

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

/*
* @brief Calculate the front vector of the camera & assign it to the global camera struct
*/
void calcCameraFront(Camera* camera, float xpos, float ypos){
            float yaw = -90.0f;
            float pitch = 0.0f;
            float lastX = 400, lastY = 300;
            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top

            const float sensitivity = 0.1f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            yaw   += xoffset;
            pitch += yoffset; 

            if(pitch > 89.0f)
                pitch =  89.0f;
            if(pitch < -89.0f)
                pitch = -89.0f;

            vec3 direction;
            direction[0] = cos(deg2rad(yaw)) * cos(deg2rad(pitch));
            direction[1] = sin(deg2rad(pitch));
            direction[2] = sin(deg2rad(yaw)) * cos(deg2rad(pitch));
            vec3_norm(camera->front, direction);
}

void handleKeyInput(AppState *app){
         
            processKeyInput(app->event.key.keysym.sym, app->currentSceneMode);
            
            const char *key = getKeyName(app->event.key.keysym.sym);
           
            if(strcmp(key, "C") == 0) {
                app->views.full.clearColor.r = randFloat(0.0,1.0);
                app->views.full.clearColor.g = randFloat(0.0,1.0);
                app->views.full.clearColor.b = randFloat(0.0,1.0);

                for(int i = 0; i < MAX_ENTITIES; i++){
                    if(app->entities[i].alive == 1 && app->entities[i].lightComponent->active == 1){
                        app->entities[i].lightComponent->ambient.r = app->views.full.clearColor.r;
                        app->entities[i].lightComponent->ambient.g = app->views.full.clearColor.g;
                        app->entities[i].lightComponent->ambient.b = app->views.full.clearColor.b;
                    }
                }
                //glClearColor(randFloat(0.0,1.0),randFloat(0.0,1.0),randFloat(0.0,1.0), 1.0);
            }
            if(strcmp(key, "T") == 0) {
                app->culling = !app->culling;
            }
            if(strcmp(key, "L") == 0) {
                // Pressing L will write all drawcalls of a frame to disk as a pngs
                app->debugDrawCalls = true;
                printf("Debug drawcalls enabled\n");
            }
            if(strcmp(key, "B") == 0) {
                app->drawBoundingBoxes = !app->drawBoundingBoxes;
            }
            if(strcmp(key, "Escape") == 0) {
                app->running = 0;
            }
            if(strcmp(key, "F") == 0) {
                toggleFullscreen(app);
            }
            if(strcmp(key, "W") == 0){
                app->views.main.camera->position[0] += app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[0];
                app->views.main.camera->position[1] += app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[1];
                app->views.main.camera->position[2] += app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[2];

                // set view matrix needs update flag
                app->views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "S") == 0){
                app->views.main.camera->position[0] -= app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[0];
                app->views.main.camera->position[1] -= app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[1];
                app->views.main.camera->position[2] -= app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[2];

                // set view matrix needs update flag
                app->views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "A") == 0){

                // Do a cross product to get the right vector for strafing

                // allocate memory for calculation.
                float newPos[3];
                float crossProduct[3];

                // copy position into newPos
                memcpy(newPos, app->views.main.camera->position, sizeof(newPos));

                // cross product
                vec3_mul_cross(crossProduct, app->views.main.camera->front, app->views.main.camera->up);

                // normalize
                vec3_norm(newPos, crossProduct);

                // move & assign new position
                app->views.main.camera->position[0] -= app->views.main.camera->speed * app->delta_time * newPos[0];
                app->views.main.camera->position[1] -= app->views.main.camera->speed * app->delta_time * newPos[1];
                app->views.main.camera->position[2] -= app->views.main.camera->speed * app->delta_time * newPos[2];

                // set view matrix needs update flag
                app->views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "D") == 0){

                // Do a cross product to get the right vector for strafing

                // allocate memory for calculation.
                float newPos[3];
                float crossProduct[3];

                // copy position into newPos
                memcpy(newPos, app->views.main.camera->position, sizeof(newPos));

                // cross product
                vec3_mul_cross(crossProduct, app->views.main.camera->front, app->views.main.camera->up);

                // normalize
                vec3_norm(newPos, crossProduct);

                // move & assign new position
                app->views.main.camera->position[0] += app->views.main.camera->speed * app->delta_time * newPos[0];
                app->views.main.camera->position[1] += app->views.main.camera->speed * app->delta_time * newPos[1];
                app->views.main.camera->position[2] += app->views.main.camera->speed * app->delta_time * newPos[2];

                // set view matrix needs update flag
                app->views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "O") == 0){
                // Change the camera to orthographic
                app->views.main.camera->isOrthographic = 1;
                app->views.main.camera->projectionMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "P") == 0){
                // Change the camera to perspective
                app->views.main.camera->isOrthographic = 0;
                app->views.main.camera->projectionMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "I") == 0){
                if(app->views.main.camera->mode == CAMERAMODE_ORBITAL){
                    app->views.main.camera->mode = CAMERAMODE_FPS;
                }else{
                    app->views.main.camera->mode = CAMERAMODE_ORBITAL;
                    app->views.main.camera->target[0] = 0.0f;
                    app->views.main.camera->target[1] = 0.0f;
                    app->views.main.camera->target[2] = 0.0f;
                }
            }
            if(strcmp(key, "U") == 0){
                app->blinnMode = !app->blinnMode;
            }
            if(strcmp(key, "G") == 0){
                app->gamma = !app->gamma;
            }
            if(strcmp(key, "H") == 0){
                app->showDepthMap = !app->showDepthMap;
            }
            if(strcmp(key, "U") == 0){
                app->showUI = !app->showUI;
            }
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

void update(AppState *app){

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
    cameraSystem(app);
    processEvents(app);
    uiPositionSystem(app);
    uiInputSystem(app);
    hoverAndClickSystem(app);
    uiSliderSystem(app);
    uiCheckboxSystem(app);
    textCursorSystem(app);
    movementSystem(app);
    modelSystem(app);
    app->prevMouseLeftDown = app->mouseLeftButtonPressed;
}

void render(AppState *app){

    // Clear the entire window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // Set culling
     if(app->culling){
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CCW);
    }else {
        glDisable(GL_CULL_FACE);
    } 

    // Render without ui on wasm
    #ifdef __EMSCRIPTEN__
    setViewport(app->views.full);
     for(int i = 0; i < MAX_ENTITIES; i++) {
        if(app->entities[i].alive == 1) {
            if(app->entities[i].meshComponent->active == 1) { // && app->entities[i].uiComponent->active == 0
                Color* diff = &app->entities[i].materialComponent->diffuse;
                Color* amb = &app->entities[i].materialComponent->ambient;
                Color* spec = &app->entities[i].materialComponent->specular;
                GLfloat shin = app->entities[i].materialComponent->shininess;
                GLuint diffMap = app->entities[i].materialComponent->diffuseMap;
                bool useDiffMap = app->entities[i].materialComponent->useDiffuseMap;
                renderMesh(app, app->entities[i].meshComponent->gpuData,app->entities[i].transformComponent,diff,amb,spec,shin,diffMap,app->views.main.camera,useDiffMap);
            }
        }
    }
    #else
   // Native/Desktop
   
   // Render depth map
   // TODO: GL_CULL_FACE to avoid Peter panning?
   if(app->shadows){
    createLightSpace(app);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, app->depthMapBuffer.FBO);
    
    glEnable(GL_DEPTH_TEST);
        for(int i = 0; i < MAX_ENTITIES; i++) {
            if(app->entities[i].alive == 1) {
                if(app->entities[i].meshComponent->active == 1) {
                    if(app->entities[i].uiComponent->active != 1){
                        if(!app->entities[i].materialComponent->isPostProcessMaterial){
                            depthshadow_renderToDepthTexture(app, app->entities[i].meshComponent->gpuData,app->entities[i].transformComponent);
                        }
                    }
                }
            }
        }

    // Enable color buffer writes again
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }
  
   // Render main view & 3d objects
   setViewportAndClear(app, app->views.full);
   setFontProjection(&app->gpuFontData,app->views.full);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(app->entities[i].alive == 1 && app->entities[i].visible) {
            if(app->entities[i].meshComponent->active == 1) {
                if(app->entities[i].uiComponent->active != 1){
                   if(!app->entities[i].materialComponent->isPostProcessMaterial){
                     renderMesh(app,  app->entities[i].meshComponent->gpuData, app->entities[i].transformComponent, app->views.main.camera,app->entities[i].materialComponent );
                   }
                    
                }
            }
        }
    }
    
    // Render GL_LINES & GL_POINTS(particles)
    for(int i = 0; i < MAX_ENTITIES; i++){
        if((app->entities[i].alive == 1 && app->entities[i].lineComponent->active == 1 && app->entities[i].visible) 
        || (app->entities[i].alive == 1 && app->entities[i].pointComponent->active == 1 && app->entities[i].visible)){
            if(app->entities[i].lineComponent->active == 1){
              renderLine(app->entities[i].lineComponent->gpuData,app->entities[i].transformComponent,app->views.main.camera,app->entities[i].lineComponent->color);
            }
            if(app->entities[i].pointComponent->active == 1){
                // NOTE: point Size is not drawing anything else than 1.0 in windows/wsl2.
                // Get point size range ( use this to debug pointSize or later at init to tell support or not of pointsize )
                //GLfloat    pointSizeRange[2];
                //glGetFloatv ( GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange );
                // Print the point size range
                //printf("Point size range: min = %f, max = %f\n", pointSizeRange[0], pointSizeRange[1]);
                renderPoints(
                    app->entities[i].pointComponent->gpuData,
                    app->entities[i].transformComponent, 
                    app->views.main.camera, 
                    app->entities[i].pointComponent->color,
                    app->entities[i].pointComponent->pointSize
                );
            }
             
               
            
        }
    }

    // Render ui scene & ui objects
    if(app->showUI){
        for(int i = 0; i < MAX_ENTITIES; i++) {
            if(app->entities[i].alive == 1 && app->entities[i].visible) {
                if(app->entities[i].meshComponent->active == 1) {
                            // render ui, could be overhead with switching viewports?. profile.    
                            if(app->drawBoundingBoxes){
                                if(app->entities[i].tag == BOUNDING_BOX){
                                    renderMesh(app, app->entities[i].meshComponent->gpuData,app->entities[i].transformComponent,app->views.ui.camera, app->entities[i].materialComponent);
                                }
                            }else {
                                if(app->entities[i].uiComponent->active == 1 && app->entities[i].tag != BOUNDING_BOX){
                                    renderMesh(app, app->entities[i].meshComponent->gpuData,app->entities[i].transformComponent,app->views.ui.camera, app->entities[i].materialComponent);
                                }
                            }
                }
            }
        }
        
        // render ui text
        setFontProjection(&app->gpuFontData,app->views.ui);
        glDisable(GL_DEPTH_TEST);
        for(int i = 0; i < MAX_ENTITIES; i++) {
            if(app->entities[i].alive == 1 && app->entities[i].visible) {
                    if(app->entities[i].uiComponent->active == 1){
                        if(strlen(app->entities[i].uiComponent->text) > 0){
                        
                            // convert transform position to viewport space
                            vec2 result;
                            convertUIcoordinateToWindowcoordinates(
                                app->views.ui,
                                app->entities[i].transformComponent,
                                app->views.full.rect.height,
                                app->views.full.rect.width,
                                result);
                        
                                // align text center vertically
                                result[1] -= (float)app->characters[0].Size[1] / 4.0;
                            renderText(app, 
                                &app->gpuFontData, 
                                app->entities[i].uiComponent->text, 
                                result[0],result[1],
                                app->charScale,app->textColor);
                        }
                    }
            }
        }  
     }

   
    
   
    #endif

    // Swap the window buffers to show the new frame
    swapWindow(app);
}


#ifdef __EMSCRIPTEN__
void emscriptenLoop(void *context) {
    AppState *app = context;
    if(!app->running){
        printf("Closing sdl canvas!\n");
        quit(app);
        printf("Goodbye!\n");
        emscripten_cancel_main_loop();
        return;
    }
    input(app);
    update(app);
    render(app);
}
#endif



int main(int argc, char **argv) {
    
   /*  if(PLATFORM == "UNKNOWN") {
        printf("Platform is unknown, please set PLATFORM in the makefile\n");
        return 1;
    }
    printf("Platform is %s\n", PLATFORM); */

    AppState appState = appDefaults();

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
   
    if (!createOpenGLContext(&appState)) exit(1);
    glEnable(GL_DEPTH_TEST);

    // Tests
    /* tests_ui_coordinate_conversion();
    tests_binary_conversion();
    tests_events();
    return 0; */
    
    // Camera setup
    Camera* uiCamera = initCamera();
    uiCamera->isOrthographic = 1;
    uiCamera->aspectRatio = appState.views.ui.rect.width / appState.views.ui.rect.height;
    uiCamera->left = (float)-width/2.0f;
    uiCamera->right = (float)width/2.0f;
    uiCamera->bottom = -(float)height/2.0f;
    uiCamera->top = (float)height/2.0f;
    uiCamera->near = -10.0f; // Set the near plane distance
    uiCamera->far = 10.0f;   // Set the far plane distance
    appState.views.ui.camera = uiCamera;
    
    Camera* mainCamera = initCamera();
    mainCamera->aspectRatio = appState.views.main.rect.width / appState.views.main.rect.height;
    appState.views.main.camera = mainCamera;

    initScene(&appState);
    
    //------------------------------------------------------
    // Main loop
    // -----------------------------------------------------
    // Wasm code
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(emscriptenLoop, &appState, 0, 1);
    #else
    // native code
    while(appState.running) {
        input(&appState);
        update(&appState);
        render(&appState);
        debugSystem(&appState);
    }

    // -----------------------------------------------------
    // Quit
    // -----------------------------------------------------
    quit(&appState);
    printf("Non emscripten shutdown complete!\n");
    return 0;
    #endif
}




    
