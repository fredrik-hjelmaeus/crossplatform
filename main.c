#include <stdio.h>
#include <time.h>
#include "opengl_types.h"
#include "types.h"
#include "globals.h"
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


void initProgram(void){
    if (!initSdlWindow(&globals)) exit(1);
    initECS();
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

void handleKeyInput(void){
         
            processKeyInput(globals.event.key.keysym.sym, globals.currentSceneMode);
            
            const char *key = getKeyName(globals.event.key.keysym.sym);
           
            if(strcmp(key, "C") == 0) {
                globals.views.full.clearColor.r = randFloat(0.0,1.0);
                globals.views.full.clearColor.g = randFloat(0.0,1.0);
                globals.views.full.clearColor.b = randFloat(0.0,1.0);

                for(int i = 0; i < MAX_ENTITIES; i++){
                    if(globals.entities[i].alive == 1 && globals.entities[i].lightComponent->active == 1){
                        globals.entities[i].lightComponent->ambient.r = globals.views.full.clearColor.r;
                        globals.entities[i].lightComponent->ambient.g = globals.views.full.clearColor.g;
                        globals.entities[i].lightComponent->ambient.b = globals.views.full.clearColor.b;
                    }
                }
                //glClearColor(randFloat(0.0,1.0),randFloat(0.0,1.0),randFloat(0.0,1.0), 1.0);
            }
            if(strcmp(key, "T") == 0) {
                globals.culling = !globals.culling;
            }
            if(strcmp(key, "L") == 0) {
                // Pressing L will write all drawcalls of a frame to disk as a pngs
                globals.debugDrawCalls = true;
                printf("Debug drawcalls enabled\n");
            }
            if(strcmp(key, "B") == 0) {
                globals.drawBoundingBoxes = !globals.drawBoundingBoxes;
            }
            if(strcmp(key, "Escape") == 0) {
                globals.running = 0;
            }
            if(strcmp(key, "F") == 0) {
                toggleFullscreen(&globals);
            }
            if(strcmp(key, "W") == 0){
                globals.views.main.camera->position[0] += globals.views.main.camera->speed * globals.delta_time * globals.views.main.camera->front[0];
                globals.views.main.camera->position[1] += globals.views.main.camera->speed * globals.delta_time * globals.views.main.camera->front[1];
                globals.views.main.camera->position[2] += globals.views.main.camera->speed * globals.delta_time * globals.views.main.camera->front[2];

                // set view matrix needs update flag
                globals.views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "S") == 0){
                globals.views.main.camera->position[0] -= globals.views.main.camera->speed * globals.delta_time * globals.views.main.camera->front[0];
                globals.views.main.camera->position[1] -= globals.views.main.camera->speed * globals.delta_time * globals.views.main.camera->front[1];
                globals.views.main.camera->position[2] -= globals.views.main.camera->speed * globals.delta_time * globals.views.main.camera->front[2];

                // set view matrix needs update flag
                globals.views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "A") == 0){

                // Do a cross product to get the right vector for strafing

                // allocate memory for calculation.
                float newPos[3];
                float crossProduct[3];

                // copy position into newPos
                memcpy(newPos, globals.views.main.camera->position, sizeof(newPos));

                // cross product
                vec3_mul_cross(crossProduct, globals.views.main.camera->front, globals.views.main.camera->up);

                // normalize
                vec3_norm(newPos, crossProduct);

                // move & assign new position
                globals.views.main.camera->position[0] -= globals.views.main.camera->speed * globals.delta_time * newPos[0];
                globals.views.main.camera->position[1] -= globals.views.main.camera->speed * globals.delta_time * newPos[1];
                globals.views.main.camera->position[2] -= globals.views.main.camera->speed * globals.delta_time * newPos[2];

                // set view matrix needs update flag
                globals.views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "D") == 0){

                // Do a cross product to get the right vector for strafing

                // allocate memory for calculation.
                float newPos[3];
                float crossProduct[3];

                // copy position into newPos
                memcpy(newPos, globals.views.main.camera->position, sizeof(newPos));

                // cross product
                vec3_mul_cross(crossProduct, globals.views.main.camera->front, globals.views.main.camera->up);

                // normalize
                vec3_norm(newPos, crossProduct);

                // move & assign new position
                globals.views.main.camera->position[0] += globals.views.main.camera->speed * globals.delta_time * newPos[0];
                globals.views.main.camera->position[1] += globals.views.main.camera->speed * globals.delta_time * newPos[1];
                globals.views.main.camera->position[2] += globals.views.main.camera->speed * globals.delta_time * newPos[2];

                // set view matrix needs update flag
                globals.views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "O") == 0){
                // Change the camera to orthographic
                globals.views.main.camera->isOrthographic = 1;
                globals.views.main.camera->projectionMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "P") == 0){
                // Change the camera to perspective
                globals.views.main.camera->isOrthographic = 0;
                globals.views.main.camera->projectionMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "I") == 0){
                if(globals.views.main.camera->mode == CAMERAMODE_ORBITAL){
                    globals.views.main.camera->mode = CAMERAMODE_FPS;
                }else{
                    globals.views.main.camera->mode = CAMERAMODE_ORBITAL;
                    globals.views.main.camera->target[0] = 0.0f;
                    globals.views.main.camera->target[1] = 0.0f;
                    globals.views.main.camera->target[2] = 0.0f;
                }
            }
            if(strcmp(key, "U") == 0){
                globals.blinnMode = !globals.blinnMode;
            }
            if(strcmp(key, "G") == 0){
                globals.gamma = !globals.gamma;
            }
            if(strcmp(key, "H") == 0){
                globals.showDepthMap = !globals.showDepthMap;
            }
            if(strcmp(key, "U") == 0){
                globals.showUI = !globals.showUI;
            }
}

void displayFps(Uint32 ticks){
    globals.frameCount++;
    
    if(ticks/1000-globals.prevTick != 0){
        char str[10];
        sprintf(str,"FPS: %d", globals.frameCount);
        setWindowTitle(&globals, str);
        globals.frameCount = 0;
    }
    globals.prevTick = ticks/1000;
}

void update(void){

    // Time 
    Uint32 ticks = getTicks();

    // Cap the frame rate
    int time_to_wait = FRAME_TARGET_TIME - (ticks - last_frame_time);
    if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        delay((Uint32)time_to_wait);
    }

    // Set delta time in seconds
    globals.delta_time = (getTicks() - last_frame_time) / 1000.0f;

    displayFps(ticks);
    
    // Systems
    cameraSystem();
    processEvents();
    uiPositionSystem();
    uiInputSystem();
    hoverAndClickSystem();
    uiSliderSystem();
    uiCheckboxSystem();
    textCursorSystem();
    movementSystem();
    modelSystem();
    globals.prevMouseLeftDown = globals.mouseLeftButtonPressed;
}

void render(void){

    // Clear the entire window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // Set culling
     if(globals.culling){
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CCW);
    }else {
        glDisable(GL_CULL_FACE);
    } 

    // Render without ui on wasm
    #ifdef __EMSCRIPTEN__
    setViewport(globals.views.full);
     for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1) { // && globals.entities[i].uiComponent->active == 0
                Color* diff = &globals.entities[i].materialComponent->diffuse;
                Color* amb = &globals.entities[i].materialComponent->ambient;
                Color* spec = &globals.entities[i].materialComponent->specular;
                GLfloat shin = globals.entities[i].materialComponent->shininess;
                GLuint diffMap = globals.entities[i].materialComponent->diffuseMap;
                bool useDiffMap = globals.entities[i].materialComponent->useDiffuseMap;
                renderMesh(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,diff,amb,spec,shin,diffMap,globals.views.main.camera,useDiffMap);
            }
        }
    }
    #else
   // Native/Desktop
   
   // Render depth map
   // TODO: GL_CULL_FACE to avoid Peter panning?
   if(globals.shadows){
    createLightSpace();
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, globals.depthMapBuffer.FBO);
    
    glEnable(GL_DEPTH_TEST);
        for(int i = 0; i < MAX_ENTITIES; i++) {
            if(globals.entities[i].alive == 1) {
                if(globals.entities[i].meshComponent->active == 1) {
                    if(globals.entities[i].uiComponent->active != 1){
                        if(!globals.entities[i].materialComponent->isPostProcessMaterial){
                            depthshadow_renderToDepthTexture(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent);
                        }
                    }
                }
            }
        }

    // Enable color buffer writes again
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }
  
   // Render main view & 3d objects
   setViewportAndClear(globals.views.full);
   setFontProjection(&globals.gpuFontData,globals.views.full);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1 && globals.entities[i].visible) {
            if(globals.entities[i].meshComponent->active == 1) {
                if(globals.entities[i].uiComponent->active != 1){
                   if(!globals.entities[i].materialComponent->isPostProcessMaterial){
                     renderMesh( globals.entities[i].meshComponent->gpuData, globals.entities[i].transformComponent, globals.views.main.camera,globals.entities[i].materialComponent );
                   }
                    
                }
            }
        }
    }
    
    // Render GL_LINES & GL_POINTS(particles)
    for(int i = 0; i < MAX_ENTITIES; i++){
        if((globals.entities[i].alive == 1 && globals.entities[i].lineComponent->active == 1 && globals.entities[i].visible) 
        || (globals.entities[i].alive == 1 && globals.entities[i].pointComponent->active == 1 && globals.entities[i].visible)){
            if(globals.entities[i].lineComponent->active == 1){
              renderLine(globals.entities[i].lineComponent->gpuData,globals.entities[i].transformComponent,globals.views.main.camera,globals.entities[i].lineComponent->color);
            }
            if(globals.entities[i].pointComponent->active == 1){
                // NOTE: point Size is not drawing anything else than 1.0 in windows/wsl2.
                // Get point size range ( use this to debug pointSize or later at init to tell support or not of pointsize )
                //GLfloat    pointSizeRange[2];
                //glGetFloatv ( GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange );
                // Print the point size range
                //printf("Point size range: min = %f, max = %f\n", pointSizeRange[0], pointSizeRange[1]);
                renderPoints(
                    globals.entities[i].pointComponent->gpuData,
                    globals.entities[i].transformComponent, 
                    globals.views.main.camera, 
                    globals.entities[i].pointComponent->color,
                    globals.entities[i].pointComponent->pointSize
                );
            }
             
               
            
        }
    }

    // Render ui scene & ui objects
    if(globals.showUI){
        for(int i = 0; i < MAX_ENTITIES; i++) {
            if(globals.entities[i].alive == 1 && globals.entities[i].visible) {
                if(globals.entities[i].meshComponent->active == 1) {
                            // render ui, could be overhead with switching viewports?. profile.    
                            if(globals.drawBoundingBoxes){
                                if(globals.entities[i].tag == BOUNDING_BOX){
                                    renderMesh(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,globals.views.ui.camera, globals.entities[i].materialComponent);
                                }
                            }else {
                                if(globals.entities[i].uiComponent->active == 1 && globals.entities[i].tag != BOUNDING_BOX){
                                    renderMesh(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,globals.views.ui.camera, globals.entities[i].materialComponent);
                                }
                            }
                }
            }
        }
        
        // render ui text
        setFontProjection(&globals.gpuFontData,globals.views.ui);
        glDisable(GL_DEPTH_TEST);
        for(int i = 0; i < MAX_ENTITIES; i++) {
            if(globals.entities[i].alive == 1 && globals.entities[i].visible) {
                    if(globals.entities[i].uiComponent->active == 1){
                        if(strlen(globals.entities[i].uiComponent->text) > 0){
                        
                            // convert transform position to viewport space
                            vec2 result;
                            convertUIcoordinateToWindowcoordinates(
                                globals.views.ui,
                                globals.entities[i].transformComponent,
                                globals.views.full.rect.height,
                                globals.views.full.rect.width,
                                result);
                        
                                // align text center vertically
                                result[1] -= (float)globals.characters[0].Size[1] / 4.0;
                            renderText(
                                &globals.gpuFontData, 
                                globals.entities[i].uiComponent->text, 
                                result[0],result[1],
                                globals.charScale,globals.textColor);
                        }
                    }
            }
        }  
     }

   
    
   
    #endif

    // Swap the window buffers to show the new frame
    swapWindow(&globals);
}


#ifdef __EMSCRIPTEN__
void emscriptenLoop() {
    if(!globals.running){
        printf("Closing sdl canvas!\n");
        quit(&globals);
        printf("Goodbye!\n");
        emscripten_cancel_main_loop();
    }
    input(&globals);
    update();
    render();
}
#endif



int main(int argc, char **argv) {
    
   /*  if(PLATFORM == "UNKNOWN") {
        printf("Platform is unknown, please set PLATFORM in the makefile\n");
        return 1;
    }
    printf("Platform is %s\n", PLATFORM); */

    // Initialize Memory Arenas
    arena_initMemory(&globals.assetArena, ASSET_MEMORY_SIZE * sizeof(Vertex));
    arena_initMemory(&globals.uiArena, UI_MEMORY_SIZE * sizeof(char));
   
    // Sub memory allocations
    globals.materials = arena_Alloc(&globals.assetArena, globals.materialsCapacity * sizeof(Material));
    // BELOW NOT YET IMPLEMENTED
    //globals.textures = arena_Alloc(&globals.assetArena, globals.texturesCapacity * sizeof(Texture));
    //globals.meshes = arena_Alloc(&globals.assetArena, globals.meshCapacity * sizeof(MeshComponent));
    //globals.groups/models = arena_Alloc(&globals.assetArena, globals.modelsCapacity * sizeof(GroupComponent));
    //globals.cameras = arena_Alloc(&globals.assetArena, globals.camerasCapacity * sizeof(Camera));
    //globals.ui_elements = arena_Alloc(&globals.assetArena, globals.uiElementsCapacity * sizeof(UIComponent));
        
    //obj_runTests();
    
    // Initialize the random number generator
    srand((unsigned int)time(NULL));
    
    initProgram();
   
    if (!createOpenGLContext(&globals)) exit(1);
    glEnable(GL_DEPTH_TEST);

    // Tests
    /* tests_ui_coordinate_conversion();
    tests_binary_conversion();
    tests_events();
    return 0; */
    
    // Camera setup
    Camera* uiCamera = initCamera();
    uiCamera->isOrthographic = 1;
    uiCamera->aspectRatio = globals.views.ui.rect.width / globals.views.ui.rect.height;
    uiCamera->left = (float)-width/2.0f;
    uiCamera->right = (float)width/2.0f;
    uiCamera->bottom = -(float)height/2.0f;
    uiCamera->top = (float)height/2.0f;
    uiCamera->near = -10.0f; // Set the near plane distance
    uiCamera->far = 10.0f;   // Set the far plane distance
    globals.views.ui.camera = uiCamera;
    
    Camera* mainCamera = initCamera();
    mainCamera->aspectRatio = globals.views.main.rect.width / globals.views.main.rect.height;
    globals.views.main.camera = mainCamera;

    initScene(&globals);
    
    //------------------------------------------------------
    // Main loop
    // -----------------------------------------------------
    // Wasm code
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscriptenLoop, 0, 1);
    #else
    // native code
    while(globals.running) {
        input(&globals);
        update();
        render();
        debugSystem();
    }

    // -----------------------------------------------------
    // Quit
    // -----------------------------------------------------
    quit(&globals);
    printf("Non emscripten shutdown complete!\n");
    return 0;
    #endif
}




    
