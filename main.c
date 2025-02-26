#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
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
#include "api.h"
#include "assets.h"


// Stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Prototypes
void createPoint(vec3 position);
void onButtonClick();
void togglePanel();
void onTextInputChange();
void createPoints(GLfloat* positions,int numPoints, Entity* entity);
Camera* initCamera();

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

//------------------------------------------------------
// Global variables / Initialization
//------------------------------------------------------

struct Globals globals = {
    .window=NULL,
    .renderer=NULL,
    .event={0},
    .gl_context=NULL,
    .running=1,
    .entities=NULL,
    .vertex_count=0,
    .delta_time=0.0f,
    .overideDrawMode=GL_TRIANGLES,
    .overrideDrawModeBool=0,
    .views = {
        .ui={{0,   0, width, height}, {0.0f, 1.0f, 0.0f, 1.0f},NULL,false},
        .main={{0, 0, width, height}, {1.0f, 0.0f, 0.0f, 1.0f},NULL,false},
        .full={{0, 0, width, height}, {0.0f, 0.0f, 0.0f, 1.0f} ,NULL,false},
    },

    // Mouse
    .firstMouse=1,
    .mouseXpos=0.0f,
    .mouseYpos=0.0f,
    .mouseLeftButtonPressed=false, // mouse down check every frame
    .prevMouseLeftDown=false,
    .mouseDoubleClick=false,
    .deselectCondition=false,
    .mouseDragged=false,
    .mouseDragStart={-10000.0f,-10000.0f},
    .mouseDragPreviousFrame={0.0f,0.0f},

    .drawBoundingBoxes=false,
    .render=true,
    .gpuFontData={0,0,0,0,0,0,GL_TRIANGLES},
    .charScale=0.5f,
    .fontSize=26,
    .textColor={187.0/255.0,188.0/255.0,196.0/255.0,1.0},
    .unitScale=100.0f, // 1 unit = 1 cm 
    .culling=false,
    .drawCallsCounter=0,
    .debugDrawCalls=false,
    //.assetArena=NULL,
    .materials=NULL,
    .materialsCount=0,
    .materialsCapacity=15,
    .objDataCapacity=20000,
    .lights={{0}},
    .lightsCount=0,
    .focusedEntityId=-1,
    .blinnMode=false,
    .gamma=false,
    .depthMapBuffer={0},
    .frameBuffer={0},
    .depthMap=0,
    .depthCubemap=0,
    .lightSpaceMatrix={{0}},
    .postProcessBuffer={0},
    .showDepthMap=false,
    .shadowWidth=256,
    .shadowHeight=256,

    // Cursor
    .cursorEntityId=-1,
    .cursorBlinkTime=0.5f,
    .cursorSelectionActive=false,
    .cursorDragStart=-1.0f,
    .cursorTextSelection={0,0},

 
    
};

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

void setRenderDrawColor(int r,int g, int b, int a) {
    // Initial renderer color
    SDL_SetRenderDrawColor(globals.renderer, r, g, b, a);
}

int pollEvent(){
    return SDL_PollEvent(&globals.event);
}

#ifdef __EMSCRIPTEN__
void setVersion() {
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES) != 0){
        printf("Failed to set OpenGL ES profile: %s\n", SDL_GetError());
    }
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) != 0){
        printf("Failed to set OpenGL ES major version: %s\n", SDL_GetError());
    }
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0) != 0){
        printf("Failed to set OpenGL ES minor version: %s\n", SDL_GetError());
    }
} 
#else
void setVersion() {
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) != 0){
        printf("Failed to set OpenGL core profile: %s\n", SDL_GetError());
    }
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) != 0){
        printf("Failed to set OpenGL major version: %s\n", SDL_GetError());
    }
    if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) != 0){
        printf("Failed to set OpenGL minor version: %s\n", SDL_GetError());
    }
}
#endif

void initWindow() {
    // Initialize SDL
    CHECK_SDL_ERROR(SDL_Init(SDL_INIT_VIDEO) != 0, SDL_GetError());

    // Set OpenGL version (3.0) or webgl won't be 2.0
    setVersion();

    // Create an SDL window
    globals.window = SDL_CreateWindow(
        "FH Engine", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        width, 
        height, 
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_RESIZABLE
    );
    CHECK_SDL_ERROR(globals.window == NULL, SDL_GetError());

}

void setViewport(struct View view) {
    glViewport(view.rect.x, view.rect.y, view.rect.width, view.rect.height);
}

void setViewportAndClear(View view){
    Rectangle rect = convertViewRectangleToSDLCoordinates(view,globals.views.full.rect.height);
   
    // Set the viewport
    glViewport(rect.x, rect.y, rect.width, rect.height);

     // Set the clear color
    glClearColor(view.clearColor.r, view.clearColor.g, view.clearColor.b, view.clearColor.a);

    // Clear the viewport
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


/**
 * @brief Set the viewport, scissor box and clear color for a view
 * Expects the view to be in opengl coordinates
 */
void setViewportWithScissorAndClear(View view) {
    Rectangle rect = convertViewRectangleToSDLCoordinates(view,globals.views.full.rect.height);
   
    // Set the viewport
    glViewport(rect.x, rect.y, rect.width, rect.height);

    // Set the scissor box
    glScissor(rect.x, rect.y, rect.width, rect.height);
    glEnable(GL_SCISSOR_TEST);

    // Set the clear color
    glClearColor(view.clearColor.r, view.clearColor.g, view.clearColor.b, view.clearColor.a);

    // Clear the viewport
    //glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Disable the scissor test
    glDisable(GL_SCISSOR_TEST);
}

/**
 * @param index
 */
void createDirectionalLightSpace(int index){
        Entity* light = &globals.entities[globals.lights[index].entityId];
        int lightSpaceIndex = globals.lights[index].lightSpaceMatrixIndex[0];
        mat4x4 lightProjection, lightView;
        float near_plane = 0.001f, far_plane = 60.0f;
        mat4x4_ortho(lightProjection, -60.0f, 60.0f, -60.0f, 60.0f, near_plane, far_plane);
        mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
        mat4x4_mul(globals.lightSpaceMatrix[lightSpaceIndex], (const float (*)[4])lightProjection, (const float (*)[4])lightView);
}

/**
 * @param index
 */
void createPointLightSpace(int index){
    Entity* light = &globals.entities[globals.lights[index].entityId];
    float near_plane = 0.001f, far_plane = 60.0f;
    mat4x4 lightProjection, lightView;
    mat4x4_perspective(lightProjection,90,1.0,near_plane,far_plane);

    vec3 targets[6] = {
        {1.0f,  0.0f,  0.0f},  // Right
        {-1.0f,  0.0f,  0.0f},  // Left
        {0.0f,  1.0f,  0.0f},  // Up
        {0.0f, -1.0f,  0.0f},  // Down
        {0.0f,  0.0f,  1.0f},  // Front
        {0.0f,  0.0f, -1.0f}   // Back
    };

    vec3 ups[6] = {
        {0.0f, -1.0f,  0.0f},  // Right
        {0.0f, -1.0f,  0.0f},  // Left
        {0.0f,  0.0f,  1.0f},  // Up
        {0.0f,  0.0f, -1.0f},  // Down
        {0.0f, -1.0f,  0.0f},  // Front
        {0.0f, -1.0f,  0.0f}   // Back
    };

    for(int i = 0; i < 6; i++){
        ASSERT(index > MAX_LIGHTS,"index larger than MAX_LIGHTS");
        int lightSpaceIndex = globals.lights[index].lightSpaceMatrixIndex[i]; 
        ASSERT(lightSpaceIndex > MAX_LIGHTSPACES,"Light space index out of bounds");

        // 6 projections with diff. view targets, up,down,left,right,front,back HERE!<--
        // front
        if(i == 0) mat4x4_look_at(lightView, light->transformComponent->position, targets[i],ups[i]);
      //  if(i == 0) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
        // back
        if(i == 1) mat4x4_look_at(lightView, light->transformComponent->position,  targets[i], ups[i]);
       // if(i == 1) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, -1.0f, 0.0f});
        // left
       // if(i == 2) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){-1.0f, 1.0f, 0.0f});
        if(i == 2) mat4x4_look_at(lightView, light->transformComponent->position,  targets[i], ups[i]);
        // right
    //    if(i == 3) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 0.0f});
        if(i == 3) mat4x4_look_at(lightView, light->transformComponent->position, targets[i], ups[i]);
        // up
       // if(i == 4) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, -1.0f});
        if(i == 4) mat4x4_look_at(lightView, light->transformComponent->position,  targets[i], ups[i]);
        // down
       // if(i == 5) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 1.0f});
        if(i == 5) mat4x4_look_at(lightView, light->transformComponent->position,  targets[i], ups[i]);

        mat4x4_mul(globals.lightSpaceMatrix[lightSpaceIndex], (const float (*)[4])lightProjection, (const float (*)[4])lightView);
    }
}

/**
 * @param index
 */
void createSpotLightSpace(int index){
    // ASSERT(true, "NOT YET IMPLEMENTED");
    Entity* light = &globals.entities[globals.lights[index].entityId];
    int lightSpaceIndex = globals.lights[index].lightSpaceMatrixIndex; 
    mat4x4 lightProjection, lightView;
    float near_plane = 0.001f, far_plane = 60.0f;
    mat4x4_ortho(lightProjection, -60.0f, 60.0f, -60.0f, 60.0f, near_plane, far_plane);
    mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    mat4x4_mul(globals.lightSpaceMatrix[lightSpaceIndex], (const float (*)[4])lightProjection, (const float (*)[4])lightView);
}

void createLightSpace(){
    for(int i = 0; i < globals.lightsCount; i++){
        if(globals.lights[i].type == DIRECTIONAL){
            createDirectionalLightSpace(i);
        }
        if(globals.lights[i].type == SPOT){
            createDirectionalLightSpace(i);
          //  createSpotLightSpace(i);
        }
        if(globals.lights[i].type == POINT){
           // createDirectionalLightSpace(i);
            createPointLightSpace(i);
        }
    }
}

void initProgram(){
    initWindow();
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

/**
 * @brief When viewport size changes, this function is called to update the UI components positions
 */
void updateUIonViewportChange(float prevWidth, float prevHeight,float ui_percentageWidth,float main_percentageWidth,float ui_percentageHeight){
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1 && globals.entities[i].uiComponent->active) {
            
            float px = globals.entities[i].transformComponent->position[0];
            float py = globals.entities[i].transformComponent->position[1];
    
            // Calculate new position
            px = px / ((prevWidth /  2.0) * ui_percentageWidth ) * ((float)globals.views.ui.rect.width  / 2.0);  
            py = py / ((prevHeight / 2.0) * ui_percentageHeight) * ((float)globals.views.ui.rect.height / 2.0); 
                                   
            globals.entities[i].transformComponent->position[0] = px;
            globals.entities[i].transformComponent->position[1] = py;
           
            // Bounding box
            globals.entities[i].boundingBoxComponent->boundingBox.min[0] = globals.entities[i].transformComponent->position[0];
            globals.entities[i].boundingBoxComponent->boundingBox.min[1] = globals.entities[i].transformComponent->position[1];
            globals.entities[i].boundingBoxComponent->boundingBox.max[0] = globals.entities[i].transformComponent->scale[0];
            globals.entities[i].boundingBoxComponent->boundingBox.max[1] = globals.entities[i].transformComponent->scale[1];

            globals.entities[i].transformComponent->modelNeedsUpdate = 1;
        }
    }
}

/**
 * @brief Recalculate the viewports when the window is resized.
 * TODO: This is fragile since the views are initialized with hardcoded pixel values 
 *       and only when resizing the window we transfer to percentage values.
 *       We also use the full view to determine what was the previous width & height of the window. 
 *       This is because when this function is called, the window has already been resized.
 *       This setup makes this function dependent on the full view beeing correct and also important
 *       to update full view correctly in this function.
 */
void recalculateViewports(int w, int h){ 
    // Recalculate viewports
    float prevHeight = (float)globals.views.full.rect.height; 
    float prevWidth = (float)globals.views.full.rect.width; 

    // MAIN VIEW
    // Calc percentage height, using old height.
    float percentageHeight = (float)globals.views.main.rect.height / prevHeight; 

    // Update main height 
    globals.views.main.rect.height = (float)h * percentageHeight; 

    // Calc percentage width, using old width.
    float main_percentageWidth = (float)globals.views.main.rect.width / prevWidth; 

    // update main width
    globals.views.main.rect.width = (float)w * main_percentageWidth;
  
    // Update main x position
    float percentageX = (float)w / prevWidth; 
    globals.views.main.rect.x = percentageX * (float)globals.views.main.rect.x; 
  
    // Update main y position
    float percentageY = (float)h / prevHeight;
    globals.views.main.rect.y = percentageY * globals.views.main.rect.y; 
    
    // UI VIEW
    // Calc percentage height, using old height.
    percentageHeight = (float)globals.views.ui.rect.height / prevHeight; 

    // Update ui height
    globals.views.ui.rect.height = (float)h * percentageHeight;

    // Calc percentage width, using old width.
    float ui_percentageWidth = (float)globals.views.ui.rect.width / prevWidth;

    // update ui width
    globals.views.ui.rect.width = (float)w * ui_percentageWidth; 

    // Update ui x position
    float ui_percentageX = (float)w / prevWidth; 
    globals.views.ui.rect.x = ui_percentageX * (float)globals.views.ui.rect.x; 

    // Update ui y position
    percentageY = (float)h / prevHeight; 
    globals.views.ui.rect.y = percentageY * (float)globals.views.ui.rect.y;

    globals.views.ui.camera->aspectRatio = (float)globals.views.ui.rect.width / globals.views.ui.rect.height;
    globals.views.ui.camera->left = (-1 * (float)globals.views.ui.rect.width)/2.0f;
    globals.views.ui.camera->right = (float)globals.views.ui.rect.width /2.0f;
    globals.views.ui.camera->bottom = (-1 * (float)globals.views.ui.rect.height)/2.0f;
    globals.views.ui.camera->top = (float)globals.views.ui.rect.height/2.0f;
    globals.views.ui.camera->projectionMatrixNeedsUpdate = 1; 
    updateUIonViewportChange(prevWidth,prevHeight ,ui_percentageWidth,main_percentageWidth,percentageHeight);
    
    // update full view ( this should be done last in this fn, since full view is used to calculate new views)
    globals.views.full.rect.height = h;
    globals.views.full.rect.width = w;
    
    // Update the projection on the perspective main view camera
    globals.views.main.camera->projectionMatrixNeedsUpdate = 1;
}

void handleKeyInput(){
         
            const char *key = SDL_GetKeyName(globals.event.key.keysym.sym);
           
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
                Uint32 windowFlags = SDL_GetWindowFlags(globals.window);
                if(windowFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                    SDL_SetWindowFullscreen(globals.window, 0);
                } else {
                    SDL_SetWindowFullscreen(globals.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
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
}

/**
 * Atm we handle all events right here. Eventually we wan't to store the polled events in a queue and process them in the update loop instead.
 * This is because this fn will become too large otherwise,and we wan't to group & handle events in a more structured way. 
 * For example, window resize events in perhaps a windowSystem, input affecting camera in a cameraSystem etc.
 */
void input() {
    
    // Process events
    while(pollEvent()) {
        
        if(globals.event.type == SDL_QUIT) {
            globals.running = 0;
        } 
            if (globals.event.type == SDL_MOUSEBUTTONDOWN) {
            
            // A button was pressed
            if (globals.event.button.button == SDL_BUTTON_LEFT) {
               
                if(globals.event.button.clicks == 2){
                   
                    globals.mouseDoubleClick = true;
                }
                
                // Left Button Pressed
                globals.mouseLeftButtonPressed = true;
                
            } else if (globals.event.button.button == SDL_BUTTON_RIGHT) {
                
              
            }
        } 
        if(globals.event.type == SDL_KEYDOWN) {
            // If input element is focused, we don't handle keyboard events here but in the uiInputSystem
            if(globals.focusedEntityId != -1){
                return;
            }else {
                handleKeyInput();
            }
        }
            
        if(globals.event.type == SDL_WINDOWEVENT && globals.event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
            int w, h; 
            SDL_GetWindowSize(globals.window, &w, &h);
            recalculateViewports(w,h);
            
            // TODO: use this for reprojection later: float aspect = (float)w / (float)h;
            //glViewport(0, 0, w / 2, h);
        }
       
        if (globals.event.type == SDL_MOUSEBUTTONUP) {
           // A button was released
           globals.mouseLeftButtonPressed = false;
           
           if(globals.cursorSelectionActive){
                globals.deselectCondition = true;
           }
           globals.mouseDragged = false;
           if(globals.focusedEntityId != -1 && globals.entities[globals.focusedEntityId].uiComponent->type == UITYPE_SLIDER){
                globals.focusedEntityId = -1;
           }
           
            
        }
        if (globals.event.type == SDL_MOUSEMOTION) {
            if(globals.mouseLeftButtonPressed){
                globals.mouseDragged = true;
            }

            int xpos = globals.event.motion.x;
            int ypos = globals.event.motion.y;

            globals.mouseXpos = (float)xpos;
            globals.mouseYpos = (float)ypos;

            // -----------TEMP CODE------------
            //printf("Mouse SDL coords: %d, %d\n", xpos, ypos);
            // mouse move in ndc coordinates
              /* float x_ndc = (2.0f * xpos) / width - 1.0f;
              float y_ndc = 1.0f - (2.0f * ypos) / height;
              float x_ui =  (-1 * x_ndc) * ((float)width * 0.5);
              float y_ui =   y_ndc * ((float)height * 0.5);  */
              //printf("Mouse moved to NDC %f, %f\n", x_ndc, y_ndc);
            //printf("Mouse moved to %f, %f\n", globals.mouseXpos, globals.mouseYpos);
            //SDLVector2 sdlMouseCoords = {xpos, ypos};
           // UIVector2 uiCoords = convertSDLToUI(sdlMouseCoords, width, height);
           //printf("Mouse UI coords: %f, %f\n", uiCoords.x, uiCoords.y);
            // END TEMP CODE------------------

            if(isPointInsideRect(globals.views.main.rect, (vec2){xpos, ypos})){ 
               // printf("Mouse is within main view\n ");
                // Update the camera front vector
                calcCameraFront(globals.views.main.camera,xpos, ypos);
                // set view matrix needs update flag (so that we recalculate the view matrix with the new front vector)
                globals.views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(isPointInsideRect(globals.views.ui.rect, (vec2){xpos, ypos})){
                globals.views.ui.isMousePointerWithin = true;
            }else{
                globals.views.ui.isMousePointerWithin = false;
            }

           
        }
    }
}

static int frameCount = 0;
static Uint32 prevTick = 0;

void update(){

    // Time 
    Uint32 ticks = SDL_GetTicks();

    // Cap the frame rate
    int time_to_wait = FRAME_TARGET_TIME - (ticks - last_frame_time);
    if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    // Frames per second
    frameCount++;

    // Set delta time in seconds
    globals.delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
    if(ticks/1000-prevTick != 0){
        
       // printf("frameCount: %d \n",frameCount);
        char str[10];
        sprintf(str,"FPS: %d", frameCount);
        //printf("The number as a string is: %s\n", str);
        SDL_SetWindowTitle(globals.window, str);
        frameCount = 0;
      //  printf("new second: %u \n",(Uint32)globals.delta_time);
    }
    prevTick = ticks/1000;
  // printf("deltatime: %f \n",globals.delta_time);
  // printf("ticks: %u \n",ticks);
  // printf("ticks/1000: %u \n",ticks/1000);
    
   

    // Systems
    cameraSystem();
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

void render(){

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

   
    
   
    #endif

    // Swap the window buffers to show the new frame
    SDL_GL_SwapWindow(globals.window); 
}

void quit(){
    // Release resources
    SDL_GL_DeleteContext(globals.gl_context);
    SDL_DestroyRenderer(globals.renderer);
    SDL_DestroyWindow(globals.window);
    SDL_Quit();
}

void initOpenGLWindow(){
  // Create an OpenGL context associated with the window.
   globals.gl_context = SDL_GL_CreateContext(globals.window);
   if (!globals.gl_context) {
      printf("Failed to create OpenGL context: %s\n", SDL_GetError());
      exit(1);
   }
   glEnable(GL_DEPTH_TEST);
}

/**
 * @brief Initialize the font
 * Load texture,setup mesh,setup material
 * After this the projection matrix for the font need to be set using setFontProjection.
 * And then you can render text using renderText.
 */
void initFont(){
    setupFontTextures("./Assets/ARIAL.TTF",globals.fontSize);
    setupFontMesh(&globals.gpuFontData);
    setupMaterial(&globals.gpuFontData,"shaders/text_vertex.glsl", "shaders/text_fragment.glsl");
}

#ifdef __EMSCRIPTEN__
void emscriptenLoop() {
    if(!globals.running){
        printf("Closing sdl canvas!\n");
        quit();
        printf("Goodbye!\n");
        emscripten_cancel_main_loop();
    }
    input();
    update();
    render();
}
#endif



/**
 * @brief Initialize the scene
 * Create the 3d and ui scene objects
 * 3d objects put into main viewport will be in an 3d-enviroment and treated normally.
 * objects created in the ui viewport will be in a 2d-environment and act more like a web ui element.
*/
void initScene(){

   globals.views.main.camera->position[1] = 35.0f;
   globals.views.main.camera->position[2] = 35.0f;
   globals.views.main.camera->target[0] = 0.0f;
   globals.views.main.camera->target[1] = 0.0f;
   globals.views.main.camera->target[2] = 0.0f;
    
   

    // Assets
    initFont();
   
    setupMaterial(&globals.depthMapBuffer, "shaders/depthMapBuffer_vert.glsl", "shaders/depthMapBuffer_frag.glsl");
  
    depthshadow_createFrameBuffer(&globals.depthMapBuffer);
    depthshadow_createDepthTexture();
    depthshadow_createDepthCubemap();
  //  depthshadow_configureFrameBuffer(&globals.depthMapBuffer);
    //depthshadow_configureCubeMapFrameBuffer(&globals.depthMapBuffer);
    initAssets();

  
 //  TextureData oldBricksTest = loadTexture("./Assets/oldbricks.jpg");
  


   // ObjGroup* truck = obj_loadFile("./Assets/truck.obj"); // Not supported atm, need .obj group support.
  // ObjGroup* cornell_box = obj_loadFile("./Assets/cornell_box.obj");  
   // ObjGroup* bunny = obj_loadFile("./Assets/bunny2.obj");
  /*  ObjGroup* plane = obj_loadFile("./Assets/plane.obj"); 
    ObjGroup* objExample = obj_loadFile("./Assets/Two_adjoining_squares_with_vertex_normals.obj");
    ObjGroup* sphere = obj_loadFile("./Assets/blender_sphere3.obj");
    ObjGroup* triangleVolumes = obj_loadFile("./Assets/triangle_volumes.obj");
    ObjGroup* teapot = obj_loadFile("./Assets/teapot.obj");*/
  //  ObjGroup* dragon = obj_loadFile("./Assets/dragon.obj");   
   // ObjGroup* textured_objects = obj_loadFile("./Assets/textured_objects.obj");
    ObjGroup* skp_arbetsrum = obj_loadFile("./Assets/arbetsrum_FINAL.obj");
 
  
   

   
    // Main viewport objects (3d scene) x,y,z coords is a world space coordinate (not yet implemented?).
  
  /*   createObject(&cornell_box->objData[0],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[1],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[2],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[3],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[4],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[5],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[6],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[7],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});   */
 
   
   // createPoint((vec3){-5.0f, -5.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&plane->objData[0],(vec3){5.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
   /*  for(int i = 0; i < 500; i++){
        createObject(&truck->objData[i],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    } */
  
    createObject(&skp_arbetsrum->objData[0],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&skp_arbetsrum->objData[1],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&skp_arbetsrum->objData[2],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&skp_arbetsrum->objData[3],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
  
  /*   createObject(&truck->objData[0],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[1],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[2],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[3],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[4],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[5],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[6],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[7],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[8],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[9],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); */
    
    //createObject(VIEWPORT_MAIN,&objExample->objData[0],(vec3){5.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&sphere->objData[0],(vec3){3.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&triangleVolumes->objData[0],(vec3){4.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&teapot->objData[0],(vec3){0.0f, 0.0f, 0.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){-90.0f, 0.0f, 0.0f}); 
   // createObject(&dragon->objData[0],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});   
   // createObject(&bunny->objData[0],(vec3){6.0f, 0.0f, 0.0f}, (vec3){10.0f, 10.0f, 10.0f}, (vec3){0.0f, 0.0f, 0.0f});   
 //   createObject(&textured_objects->objData[0],(vec3){2.0f, 1.0f, -6.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});  
   // createObject(&textured_objects->objData[1],(vec3){2.0f, 1.0f, -6.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});     
    

    // lights
    createLight(lightMaterial,(vec3){0.0f,5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){0.0f, -1.0f, 0.0f},DIRECTIONAL);
    createLight(lightMaterial,(vec3){-1.0f, 10.0f, 1.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},SPOT);
    createLight(lightMaterial,(vec3){0.25f, 3.5f, 0.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){0.0f, -1.0f, 0.0f},SPOT);
    createLight(lightMaterial,(vec3){3.0f, 2.0f, 1.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},SPOT);
    createLight(lightMaterial,(vec3){0.0f, 1.0f, -5.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},SPOT);
    createLight(lightMaterial,(vec3){10.7f, 1.2f, 5.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);
    createLight(lightMaterial,(vec3){10.7f, -5.2f, -3.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);
    createLight(lightMaterial,(vec3){-12.7f, 5.2f, 2.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);
    createLight(lightMaterial,(vec3){-8.7f, 0.2f, -5.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);    

    // Primitives
    createPlane(objectMaterial, (vec3){0.0f, -1.0f, 0.0f}, (vec3){50.0f, 50.0f, 50.0f}, (vec3){-90.0f, 0.0f, 0.0f});
    //createCube(objectMaterial,(vec3){10.0f, 3.0f, 12.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
  
    // Frame buffer quad
    ui_createRectangle(depthMapMaterial, (vec3){0.0f, 0.0f,5.0f}, (vec3){800.0f, 600.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},NULL);

    debug_drawFrustum();

   // UI scene objects creation (2d scene) x,y coords where x = 0 is left and y = 0 is top and x,y is pixel positions. (controlled by the uiPositionSystem)
   // Scale is in pixels, 100.0f is 100 pixels etc.
   // z position will be z-depth, much like in DOM in web.Use this to control draw order.
   // TODO: implement rotation, it is atm not affecting. 
 
    // UI Settings Panel
    Entity* settingsPanel = ui_createPanel(uiBoundingBoxMat,(vec3){545.0f, 5.0f, 3.0f}, (vec3){250.0f, 350.0f, 1.0f},(vec3){0.0f, 0.0f, 0.0f},NULL);

    ui_createButton(flatColorUiGrayMat, (vec3){545.0f, 5.0f, 0.0f}, (vec3){250.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Header", togglePanel, settingsPanel);

    // Row 1 Text Input with TextField as label field.
    ui_createTextInput(textInputUiMat, (vec3){680.0f, 35.0f, 1.0f}, (vec3){110.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "TextInput",onTextInputChange,settingsPanel);
    ui_createTextField(flatColorUiDarkGrayMat, (vec3){555.0f, 35.0f, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "TextField",settingsPanel);
    // Row 2 Slider
    ui_createTextField(flatColorUiDarkGrayMat, (vec3){555.0f, 65.0f, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Slider Text Field",settingsPanel);
    ui_createSlider(flatColorUiGrayMat,textInputUiMat, (vec3){650.0f, 65.0f, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},settingsPanel);
    ui_createTextInput(textInputUiMat, (vec3){730.0f, 65.0f, 1.0f}, (vec3){60.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "1.00",onTextInputChange,settingsPanel);
    // Row 3 Checkbox
    ui_createTextField(flatColorUiGrayMat, (vec3){555.0f, 100.0f, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Turn off all spot & point lights",settingsPanel);
    ui_createCheckbox(flatColorUiGrayMat,flatColorUiDarkGrayMat, (vec3){650.0f, 100.0f, 1.0f}, (vec3){20.0f, 20.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},settingsPanel);
    // Row 4 Checkbox
    ui_createTextField(flatColorUiGrayMat, (vec3){555.0f, 135.0f, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Turn off shadows",settingsPanel);
    ui_createCheckbox(flatColorUiGrayMat,flatColorUiDarkGrayMat, (vec3){650.0f, 135.0f, 1.0f}, (vec3){20.0f, 20.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},settingsPanel);
   

    // Row 4 List
    float yPos = 135.0f;
    for(int i=0; i < MAX_LIGHTS; i++){
        yPos += 35.0f;
        char* lightname = "test"; // globals.entities[globals.lights[i]].id;
        ui_createTextField(flatColorUiGrayMat, (vec3){555.0f, yPos, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, lightname , settingsPanel);
    }
 
    ui_createRectangle(flatColorUiGrayMat, (vec3){545.0f, 30.0f, 0.0f}, (vec3){5.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},settingsPanel);
    ui_createRectangle(flatColorUiDarkGrayMat, (vec3){550.0f, 30.0f, 1.0f}, (vec3){245.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},settingsPanel);

    // Bottom panel
    ui_createButton(flatColorUiGrayMat, (vec3){545.0f, 355.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Header2",onButtonClick,NULL);
    ui_createRectangle(flatColorUiGrayMat, (vec3){545.0f, 405.0f, 0.0f}, (vec3){10.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},NULL);
    ui_createRectangle(flatColorUiDarkGrayMat, (vec3){555.0f, 405.0f, 1.0f}, (vec3){240.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},NULL);   
    
   

    //ui_slider(flatColorUiDarkGrayMat, (vec3){545.0f, 55.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Slider",onSliderChange);
    //ui_colorPicker(flatColorUiDarkGrayMat, (vec3){545.0f, 105.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "ColorPicker",onColorPickerChange);
    //ui_checkbox
    //ui_open_file (texture,obj-model,.fh-packfile,)


 //ui_createRectangle(uiMaterial, (vec3){765.0f, 5.0f, 0.0f}, (vec3){35.0f, 50.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f});

 // Textured button
 //ui_createButton(uiMaterial, (vec3){150.0f, 0.0f, 0.0f}, (vec3){150.0f, 50.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Rotate",onButtonClick);
  
   // TODO: create slider or input for ui using this
  printf("materials-list (%d): \n",globals.materialsCount);
    for(int i = 0; i < globals.materialsCount; i++){
        printf("(%d),%s \n",i,globals.materials[i].name);
   /*       printf("shininess %f \n",globals.materials[i].shininess);
        printf("ambient.r %f \n",globals.materials[i].ambient.r);
        printf("ambient.g %f \n",globals.materials[i].ambient.g);
        printf("ambient.b %f \n",globals.materials[i].ambient.b);
        printf("diffuse.r %f \n",globals.materials[i].diffuse.r);
        printf("diffuse.g %f \n",globals.materials[i].diffuse.g);
        printf("diffuse.b %f \n",globals.materials[i].diffuse.b);
        printf("specular.r %f \n",globals.materials[i].specular.r);
        printf("specular.g %f \n",globals.materials[i].specular.g);
        printf("specular.b %f \n",globals.materials[i].specular.b);
        printf("ior/Ni %f \n",globals.materials[i].ior);
        printf("alpha %f \n",globals.materials[i].alpha); 
        printf("diffuseMap %d \n",globals.materials[i].diffuseMap);
        printf("ambientMap %d \n",globals.materials[i].ambientMap);
        printf("specularMap %d \n",globals.materials[i].specularMap);
        printf("shininessMap %d \n\n",globals.materials[i].shininessMap);  */
   }   

}

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
   
    initOpenGLWindow();
    
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

    initScene();

    
    //------------------------------------------------------
    // Main loop
    // -----------------------------------------------------
    // Wasm code
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscriptenLoop, 0, 1);
    #else
    // native code
    while(globals.running) {
        input();
        update();
        render();
        debugSystem();
    }

    // -----------------------------------------------------
    // Quit
    // -----------------------------------------------------
    quit();
    printf("Non emscripten shutdown complete!\n");
    return 0;
    #endif
}

//------------------------------------------------------
//  PROGRAM ACTIONS API
//------------------------------------------------------
// TODO: Move these to api.c after we used them.
void createPoint(vec3 position){
   /*  GLfloat vertices[] = {
        position[0], position[1], position[2],  1.0f, 1.0f, 1.0f, 0.0f, 0.0f
    }; */
}
void createPoints(GLfloat* positions,int numPoints, Entity* entity){
    entity->pointComponent->active = 1;
    entity->pointComponent->color = (Color){1.0f,0.0f,1.0f,1.0f};
    entity->pointComponent->points = positions;
    entity->pointComponent->pointSize = 10.0f;
    entity->transformComponent->position[0] = positions[0];
    entity->transformComponent->position[1] = positions[1];
    entity->transformComponent->position[2] = positions[2];
    entity->transformComponent->scale[0] = 1.0f;
    entity->transformComponent->scale[1] = 1.0f;
    entity->transformComponent->scale[2] = 1.0f;
    entity->transformComponent->rotation[0] = 0.0f;
    entity->transformComponent->rotation[1] = 0.0f;
    entity->transformComponent->rotation[2] = 0.0f;
    entity->transformComponent->modelNeedsUpdate = 1;
    setupPoints(positions,numPoints,entity->pointComponent->gpuData);
    setupMaterial(entity->pointComponent->gpuData,"shaders/point_vertex.glsl", "shaders/point_fragment.glsl");
}

/**
 * TODO: This will create a ui_panel and is in turn an abstraction to create a group of ui-elements to make up this panel.
 */
void ui_panel(Material material, vec3 position,vec3 scale, vec3 rotation,Rectangle rect, bool floating, bool collapsable){
  //ui_createRectangle(Material material, vec3 positi)
  printf("not yet implemented \n");
}
void onButtonClick() {

    for(int i = 0; i < MAX_ENTITIES; i++){
       if(globals.entities[i].alive == 1 && globals.entities[i].uiComponent->active != 1 && globals.entities[i].transformComponent->active == 1){
            globals.entities[i].transformComponent->rotation[1] += 0.01f;
            globals.entities[i].transformComponent->modelNeedsUpdate = 1;
            
       }
    }
    printf("Button pressed!\n");
}
void togglePanel(){
   // printf("hello");
    BoundingBox boundary;
    for(int i = 0; i < MAX_ENTITIES; i++){
       if(globals.entities[i].alive == 1 && globals.entities[i].uiComponent->active != 1 && globals.entities[i].uiComponent->clicked){
        printf("Sclicked %d \n",globals.entities[i].id);
        
            if(globals.entities[i].uiComponent->parent != NULL){
                printf("parentId %d ",globals.entities[i].uiComponent->parent->id);
            }
       }
    }
   /*  for(int i = 0; i < MAX_ENTITIES; i++){
       if(globals.entities[i].alive == 1 && globals.entities[i].uiComponent->active == 1 && globals.entities[i].transformComponent->active == 1){
      
        if(isPointInsideRect(boundary, (vec2){ globals.entities[i].uiComponent->boundingBox.x, globals.entities[i].uiComponent->boundingBox.y}) 
        && isPointInsideRect(boundary, (vec2){ globals.entities[i].uiComponent->boundingBox.x+globals.entities[i].uiComponent->boundingBox.x 
        + globals.entities[i].uiComponent->boundingBox.width, globals.entities[i].uiComponent->boundingBox.y
        + globals.entities[i].uiComponent->boundingBox.height})){
             if(globals.entities[i].transformComponent->scale[1] > 0.0f){
                globals.entities[i].transformComponent->scale[1] -= 0.61f;
            }
            globals.entities[i].transformComponent->modelNeedsUpdate = 1;
        }

        
           
            
       }
    } */
   // printf("Panel toggled!\n");
}
void onTextInputChange(){
    printf("Text input changed!\n");
}

// -----------------------------------------------------
// ASSETS
// -----------------------------------------------------



    
