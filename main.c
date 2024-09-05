#include <stdio.h>
// macOS:
// build setup with .sh-file
// renderers:
// quartz : https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/drawingwithquartz2d/Introduction/Introduction.html
// SFML: https://github.com/SFML/CSFML
// gcc -o my_program newfile.c -I./include -L./lib -lsfml-graphics -lsfml-window -lsfml-system
// Vulkan
// Allegro
// Cairo
// opengl
// metal
// scenekit
// Core Animation
// Core Image
// SpriteKit
// imGui
// GTK
// sdl
#include <SDL2/SDL.h>
// Qt
// Raylib

#include <stdio.h>

#include "types.h"
#include "globals.h"
#include "utils.h"
#include <time.h>
#include "opengl.h"




#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Prototypes
void createTriangle(int ui,Color diffuse);
void createRectangle(int ui,Color diffuse,GLuint diffuseTextureId,vec3 position,vec3 scale,vec3 rotation);
void createCube(int ui,Color diffuse,GLuint diffuseTextureId);
void createObject(int ui,Color diffuse,GLuint diffuseTextureId,ObjData* obj);
void createLight(int ui,Color diffuse,GLuint diffuseTextureId);
Camera* initCamera();
TextureData loadTexture();

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
        .ui={{0, 400, 800, 200}, {0.0f, 0.0f, 0.0f, 1.0f},NULL,false},
        .main={{0, 0, 800, 400}, {1.0f, 0.0f, 0.0f, 1.0f},NULL,false},
        .full={{0, 0, 800, 600}, {0.0f, 0.0f, 0.0f, 1.0f},NULL,false},
    },
    .firstMouse=1,
    .mouseXpos=0.0f,
    .mouseYpos=0.0f,
    .drawBoundingBoxes=false,
    .render=true,
    .gpuFontData={0,0,0,0,0,0,GL_TRIANGLES},
    .unitScale=100.0f,
};

// Window dimensions
static const int width = 800;  // If these change, the views defaults should be changed aswell.
static const int height = 600; // If these change, the views defaults should be changed aswell.

// Colors
Color blue = {0.0f, 0.0f, 1.0f, 1.0f};
Color red = {1.0f, 0.0f, 0.0f, 1.0f};
Color green = {0.0f, 1.0f, 0.0f, 1.0f};

// Time variables
#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)
int last_frame_time = 0;


//------------------------------------------------------
//  ECS
//------------------------------------------------------

// Max entities constant
#define MAX_ENTITIES 1000

void* allocateComponentMemory(size_t componentSize, const char* componentName) {
    void* components = calloc(MAX_ENTITIES, componentSize);
    if(components == NULL) {
        printf("Failed to allocate memory for %s components\n", componentName);
        exit(1);
    }
    return components;
}

void initializeTransformComponent(TransformComponent* transformComponent){
    transformComponent->active = 0;
    transformComponent->position[0] = 0.0f;
    transformComponent->position[1] = 0.0f;
    transformComponent->position[2] = 0.0f;
    transformComponent->scale[0] = 1.0f;
    transformComponent->scale[1] = 1.0f;
    transformComponent->scale[2] = 1.0f;
    transformComponent->rotation[0] = 0.0f;
    transformComponent->rotation[1] = 0.0f;
    transformComponent->rotation[2] = 0.0f;
    transformComponent->modelNeedsUpdate = 1;
}

void initializeMatrix4(float (*matrix)[4][4]) {
    mat4x4_identity(*matrix);
}

void initializeGroupComponent(GroupComponent* groupComponent){
    groupComponent->active = 0;
    initializeMatrix4(&groupComponent->transform);
}

void initializeMeshComponent(MeshComponent* meshComponent){
    meshComponent->active = 0;
    meshComponent->vertices = NULL;
    meshComponent->vertexCount = 5;
    meshComponent->indices = NULL;
    meshComponent->indexCount = 0;
    meshComponent->gpuData = (GpuData*)malloc(sizeof(GpuData));
    if(meshComponent->gpuData == NULL) {
        printf("Failed to allocate memory for gpuData\n");
        exit(1);
    }
    // Initialize GpuData
    meshComponent->gpuData->VAO = 0;
    meshComponent->gpuData->VBO = 0;
    meshComponent->gpuData->EBO = 0;
    meshComponent->gpuData->drawMode = GL_TRIANGLES; // Default draw mode
    meshComponent->gpuData->numIndicies = 0;
}

void initializeMaterialComponent(MaterialComponent* materialComponent){
    materialComponent->active = 0;
    materialComponent->ambient.r = 0.0f;
    materialComponent->ambient.g = 0.0f;
    materialComponent->ambient.b = 0.0f;
    materialComponent->ambient.a = 1.0f;
    materialComponent->diffuse.r = 0.0f;
    materialComponent->diffuse.g = 0.0f;
    materialComponent->diffuse.b = 0.0f;
    materialComponent->diffuse.a = 1.0f;
    materialComponent->specular.r = 0.0f;
    materialComponent->specular.g = 0.0f;
    materialComponent->specular.b = 0.0f;
    materialComponent->specular.a = 1.0f;
    materialComponent->shininess = 0.0f;
    materialComponent->useDiffuseMap = true;
}

// Max text length for UI components
#define MAX_TEXT_LENGTH 100

void initializeUIComponent(UIComponent* uiComponent){
    uiComponent->active = 0;
    uiComponent->hovered = 0;
    uiComponent->clicked = 0;
    uiComponent->boundingBox.x = 0;
    uiComponent->boundingBox.y = 0;
    uiComponent->boundingBox.width = 0;
    uiComponent->boundingBox.height = 0;
    uiComponent->text = (char*)malloc(MAX_TEXT_LENGTH * sizeof(char));
    if (uiComponent->text != NULL) {
        // Initialize the allocated memory to an empty string
        uiComponent->text[0] = '\0';
    }
    uiComponent->uiNeedsUpdate = 0;
}


/**
 * @brief Initialize the ECS
 * Allocates memory pools for the components and entities
*/
void initECS(){
    // Allocate memory for MAX_ENTITIES Components structs
    TransformComponent* transformComponents = allocateComponentMemory(sizeof(TransformComponent), "transform");
    GroupComponent* groupComponents = allocateComponentMemory(sizeof(GroupComponent), "group");
    MeshComponent* meshComponents = allocateComponentMemory(sizeof(MeshComponent), "mesh");
    MaterialComponent* materialComponents = allocateComponentMemory(sizeof(MaterialComponent), "material");
    UIComponent* uiComponents = allocateComponentMemory(sizeof(UIComponent), "ui");

    // Allocate memory for MAX_ENTITIES Entity structs
    Entity* entities = (Entity*)calloc(MAX_ENTITIES, sizeof(Entity));
    if(entities == NULL) {
        printf("Failed to allocate memory for entities\n");
        exit(1);
    }

    // Initialize entities
    for (int i = 0; i < MAX_ENTITIES; i++) {
        entities[i].alive = 0;
        entities[i].id = i;
        entities[i].tag = UNINITIALIZED;
        entities[i].transformComponent = &transformComponents[i];
        initializeTransformComponent(entities[i].transformComponent);
        entities[i].groupComponent = &groupComponents[i];
        initializeGroupComponent(entities[i].groupComponent);
        entities[i].meshComponent = &meshComponents[i];
        initializeMeshComponent(entities[i].meshComponent);
        entities[i].materialComponent = &materialComponents[i];
        initializeMaterialComponent(entities[i].materialComponent); 
        entities[i].uiComponent = &uiComponents[i];
        initializeUIComponent(entities[i].uiComponent);
    }

    globals.entities = entities;
}

Entity* addEntity(enum Tag tag){
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 0) {
            globals.entities[i].alive = 1;
            globals.entities[i].tag = tag;
            return &globals.entities[i];
        }
    }
    // if we get here we are out of entities and need to increase the MAX_ENTITIES constant 
    // for now, we just exit the program
    printf("Out of entities\n");
    exit(1);
}

// remove entity
// get entity by id
// get entities by tag




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

void updateUIonViewportChange(){
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1 && globals.entities[i].uiComponent->active) {
               
  
                // Position of element on spawn:
                float ui_viewport_half_width = 600;// (float)globals.views.ui.rect.width / 2; // 400 -> 960     250 / 400 = 0.625 => 0.625 * 960 = 600
                float ui_viewport_half_height = 90;//(float)globals.views.ui.rect.height / 2; // 100 -> 180   100 / 200 = 0.5  => 0.5 * 180 = 90
                
                // Half scale of element
                float scaleFactorX = globals.entities[i].transformComponent->scale[0] / globals.unitScale;  // 1.0
                float scaleFactorY = globals.entities[i].transformComponent->scale[1] / globals.unitScale; // 0.5

                // TODO: rotation
               /*  printf("requested x %f \n", globals.entities[i].transformComponent->position[0]);
                printf("requested y %f \n", globals.entities[i].transformComponent->position[1]);
 */
                // position of element
                float requested_x = globals.entities[i].transformComponent->position[0];
                float requested_y = globals.entities[i].transformComponent->position[1];

                // move element to upper left corner and then add requested position.
               globals.entities[i].transformComponent->position[0] = -1.0 * ui_viewport_half_width + globals.unitScale / 2.0 * scaleFactorX + requested_x;
               globals.entities[i].transformComponent->position[1] = globals.unitScale - (ui_viewport_half_height / 2.0 * scaleFactorY) - requested_y;


            /*     printf(" x %f \n",  globals.entities[i].transformComponent->position[0]);
                printf(" y %f \n", globals.entities[i].transformComponent->position[1]);
                printf(" width %f \n", globals.entities[i].transformComponent->scale[0]);
                printf(" height %f \n", globals.entities[i].transformComponent->scale[1]) */;

                
                globals.entities[i].transformComponent->position[0] = 600.0f;
                globals.entities[i].transformComponent->position[1] = 90.0f;
                
                // Bounding box
                globals.entities[i].uiComponent->boundingBox.x = globals.entities[i].transformComponent->position[0];
                globals.entities[i].uiComponent->boundingBox.y = globals.entities[i].transformComponent->position[1];
                globals.entities[i].uiComponent->boundingBox.width = globals.entities[i].transformComponent->scale[0];
                globals.entities[i].uiComponent->boundingBox.height = globals.entities[i].transformComponent->scale[1];
                
                
                
                
                
                printf("-----------------entity %d \n", i);
                printf(" x %f \n",  globals.entities[i].transformComponent->position[0]);
                printf(" y %f \n", globals.entities[i].transformComponent->position[1]);
                printf(" width %f \n", globals.entities[i].transformComponent->scale[0]);
                printf(" height %f \n", globals.entities[i].transformComponent->scale[1]);
                
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
void recalculateViewports(int w, int h){ // 600 -> 1024 = 600 / 1024 = 0.5859375
    // Recalculate viewports
    float prevHeight = (float)globals.views.full.rect.height;
    float prevWidth = (float)globals.views.full.rect.width;

    // main view
    // Calc percentage height, using old height.
    float percentageHeight = (float)globals.views.main.rect.height / prevHeight; // 400 / 600 = 0.6666666666666666
   // printf("percentageHeight main: %f\n", percentageHeight);

    // Update main height 
    globals.views.main.rect.height = (float)h * percentageHeight; // 1
    printf("new main height: %d\n", globals.views.main.rect.height);

    // Calc percentage width, using old width.
    float percentageWidth = (float)globals.views.main.rect.width / prevWidth; // 800 / 800 = 1.0
   // printf("percentageWidth main: %f\n", percentageWidth);

    // update main width
    globals.views.main.rect.width = (float)w * percentageWidth;
    printf("new main width: %d\n", globals.views.main.rect.width);

    // Update main x position
    float percentageX = (float)w / prevWidth; // ex 1024 / 800 = 1.28
    globals.views.main.rect.x = percentageX * globals.views.main.rect.x; // ex 1.28 * 0 = 0 or 1.28 * 200 = 256
    printf("new main x: %d\n", globals.views.main.rect.x);

    // Update main y position
    float percentageY = (float)h / prevHeight; // ex 1024 / 600 = 1.7066666666666668
    globals.views.main.rect.y =  0.0f;//percentageY * globals.views.main.rect.y; // ex 1.7066666666666668 * 0 = 0 or 1.7066666666666668 * 200 = 341.3333333333333
    printf("new main y: %d\n", globals.views.main.rect.y);
    
    // ui view
    // Calc percentage height, using old height.
    percentageHeight = (float)globals.views.ui.rect.height / prevHeight; // 200 / 600 = 0.3333333333333333
   // printf("percentageHeight ui: %f\n", percentageHeight);

    // Update ui height
    globals.views.ui.rect.height = (float)h * percentageHeight;
    printf("new ui height: %d\n", globals.views.ui.rect.height);

    // Calc percentage width, using old width.
    percentageWidth = (float)globals.views.ui.rect.width / prevWidth; // 800 / 800 = 1.0 
   // printf("percentageWidth ui: %f\n", percentageWidth);

    // update ui width
    globals.views.ui.rect.width = (float)w * percentageWidth;
    printf("new ui width: %d\n", globals.views.ui.rect.width);

    // Update ui x position
    percentageX = (float)w / prevWidth; // ex 1024 / 800 = 1.28
    globals.views.ui.rect.x = percentageX * globals.views.ui.rect.x; // ex 1.28 * 0 = 0 or 1.28 * 200 = 256
    printf("new ui x: %d\n", globals.views.ui.rect.x);

    // Update ui y position
    percentageY = (float)h / prevHeight; // ex 1080 / 600 = 1.8
   // printf("------------------\n");
   // printf("y BEFORE: %d\n", globals.views.ui.rect.y);
    globals.views.ui.rect.y = 720.0f;//percentageY * (float)globals.views.ui.rect.y; // ex 1.8 * 0 = 0 or 1.8* 400 = 720
    printf("new ui y: %d\n", globals.views.ui.rect.y);

    globals.views.ui.camera->aspectRatio = (float)globals.views.ui.rect.width / globals.views.ui.rect.height;
    globals.views.ui.camera->left = (-1 * (float)globals.views.ui.rect.width)/2.0f;
    globals.views.ui.camera->right = (float)globals.views.ui.rect.width /2.0f;
    globals.views.ui.camera->bottom = (-1 * (float)globals.views.ui.rect.height)/2.0f;
    globals.views.ui.camera->top = (float)globals.views.ui.rect.height/2.0f;
    globals.views.ui.camera->projectionMatrixNeedsUpdate = 1; 
    updateUIonViewportChange();

    /* globals.views.ui.camera->aspectRatio = 1920.0f/1080.0f;
    globals.views.ui.camera->left = 1920.0f/2.0f;
    globals.views.ui.camera->right = 1920.0f /2.0f;
    globals.views.ui.camera->bottom = 1080.0f/2.0f;
    globals.views.ui.camera->top = 1080.0f/2.0f; */
  //  globals.views.ui.camera->projectionMatrixNeedsUpdate = 1;
    
    
    // update full view ( this should be done last in this fn, since full view is used to calculate new views)
    globals.views.full.rect.height = h;
    globals.views.full.rect.width = w;
    
    // Update the projection on the perspective main view camera
    globals.views.main.camera->projectionMatrixNeedsUpdate = 1;
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
        if(globals.event.type == SDL_KEYDOWN) {
            const char *key = SDL_GetKeyName(globals.event.key.keysym.sym);
            if(strcmp(key, "C") == 0) {
                globals.views.main.clearColor.r = randFloat(0.0,1.0);
                globals.views.main.clearColor.g = randFloat(0.0,1.0);
                globals.views.main.clearColor.b = randFloat(0.0,1.0);
        
                //glClearColor(randFloat(0.0,1.0),randFloat(0.0,1.0),randFloat(0.0,1.0), 1.0);
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
        }
        if(globals.event.type == SDL_WINDOWEVENT && globals.event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
            int w, h; 
            SDL_GetWindowSize(globals.window, &w, &h);
            printf("New Window size: %d x %d\n", w, h);
            recalculateViewports(w,h);
            
            // TODO: use this for reprojection later: float aspect = (float)w / (float)h;
            //glViewport(0, 0, w / 2, h);
        }
        if (globals.event.type == SDL_MOUSEBUTTONDOWN) {
            printf("Mouse button pressed\n");
            // A button was pressed
            if (globals.event.button.button == SDL_BUTTON_LEFT) {
                // Left Button Pressed
                printf("Left button pressed\n");
            } else if (globals.event.button.button == SDL_BUTTON_RIGHT) {
                // Left Button Released
                printf("Right button pressed\n");
            }
        }  
        if (globals.event.type == SDL_MOUSEBUTTONUP) {
            // A button was released
            printf("Mouse button released\n");
        }
        if (globals.event.type == SDL_MOUSEMOTION) {
            int xpos =  globals.event.motion.x;
            int ypos = globals.event.motion.y;

            globals.mouseXpos = (float)xpos;
            globals.mouseYpos = (float)ypos;

            // -----------TEMP CODE------------
            printf("Mouse moved to %d, %d\n", xpos, ypos);
            // mouse move in ndc coordinates
            //  float x_ndc = (2.0f * xpos) / width - 1.0f;
            //   float y_ndc = 1.0f - (2.0f * ypos) / height;
            //  printf("Mouse moved to NDC %f, %f\n", x_ndc, y_ndc);
            /*  
            .views = {
                .ui={0, 0, 800, 200, {0.0f, 1.0f, 0.0f, 1.0f}, SPLIT_HORIZONTAL, NULL,NULL},
                .main={0, 200, 800, 400, {1.0f, 0.0f, 0.0f, 1.0f}, SPLIT_DEFAULT, &globals.views.ui,NULL},
                .full={0, 0, 800, 600, {0.0f, 0.0f, 1.0f, 1.0f}, SPLIT_DEFAULT, NULL,NULL},
            }, */
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
                /* float degrees = 15.5f * globals.delta_time;
                float radians = degrees * M_PI / 180.0f;  */ 
               // printf("Mouse is within ui view\n ");
                
               
            }else{
                globals.views.ui.isMousePointerWithin = false;
            }

           
        }
    }
}

void updateCamera(Camera* camera){
    // Look for view needs update flag & update/recalc view matrix if needed.
    if(camera->viewMatrixNeedsUpdate == 1){
        // create view/camera transformation
        mat4x4 view;
        mat4x4_identity(view);

        // target / center 
        camera->target[0] = camera->position[0] + camera->front[0];
        camera->target[1] = camera->position[1] + camera->front[1];
        camera->target[2] = camera->position[2] + camera->front[2];

        // camera up
        mat4x4_look_at(view,camera->position, camera->target, camera->up);

        // Copy the view matrix to camera.view
        memcpy(camera->view, view, sizeof(mat4x4));

        camera->viewMatrixNeedsUpdate = 0;
    }

    // Look for projection needs update flag & update/recalc projection matrix if needed.
    // There are 4 scenarios where you would need to recalc. the projection matrix:
    // - Window Resize / Change in aspect ratio
    // - Camera projection type change, perspective to orthographic or vice versa
    // - Change in FOV
    // - Change in near/far plane
    if(camera->projectionMatrixNeedsUpdate == 1){ 
        mat4x4 projection;
        mat4x4_identity(projection);

        if (camera->isOrthographic) {
            // Calculate orthographic projection matrix
            mat4x4_ortho(projection, camera->left, camera->right, camera->bottom, camera->top, camera->near, camera->far);
        } else {
            // Calculate perspective projection matrix
            mat4x4_perspective(projection, camera->fov, camera->aspectRatio, camera->near, camera->far);
        }
        
        // Copy the projection matrix to camera.projection
        memcpy(camera->projection, projection, sizeof(mat4x4));

        camera->projectionMatrixNeedsUpdate = 0;
    }
}

/**
 * @brief Camera system
 * Handles camera update.
 * Atm we are not handling everything about the camera here, just the update of projection & view.
 * Movement is handled in the input function, but will eventually be moved here.
 */
void cameraSystem(){
    updateCamera(globals.views.main.camera);
    updateCamera(globals.views.ui.camera);
}

void uiSystem(){
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1 && globals.entities[i].uiComponent->active && globals.entities[i].transformComponent->active && globals.entities[i].uiComponent->uiNeedsUpdate) {
                
                // Goal here: Position element in "UI" space, where start 0,0 is the center of the ui-viewport screen. 
                // We will position the element to top left corner of the screen and treat this as 0,0 instead.
                // If position or scale changes, this calculation won't be correct anymore.
                // This calculation only works for elements that are spawned in the center of the screen.
                
                // Position of element on spawn:
                float ui_viewport_half_width = (float)globals.views.ui.rect.width / 2; 
                float ui_viewport_half_height = (float)globals.views.ui.rect.height / 2; 
                
                // Half scale of element
                float scaleFactorX = globals.entities[i].transformComponent->scale[0] / globals.unitScale;  
                float scaleFactorY = globals.entities[i].transformComponent->scale[1] / globals.unitScale; 

                // TODO: rotation
            
                // position of element
                float requested_x = globals.entities[i].transformComponent->position[0];
                float requested_y = globals.entities[i].transformComponent->position[1];

                // move element to upper left corner and then add requested position.
               globals.entities[i].transformComponent->position[0] = -1.0 * ui_viewport_half_width + globals.unitScale / 2.0 * scaleFactorX + requested_x;
               globals.entities[i].transformComponent->position[1] = globals.unitScale - (ui_viewport_half_height / 2.0 * scaleFactorY) - requested_y;  
                
                // Bounding box
                globals.entities[i].uiComponent->boundingBox.x = globals.entities[i].transformComponent->position[0];
                globals.entities[i].uiComponent->boundingBox.y = globals.entities[i].transformComponent->position[1];
                globals.entities[i].uiComponent->boundingBox.width = globals.entities[i].transformComponent->scale[0];
                globals.entities[i].uiComponent->boundingBox.height = globals.entities[i].transformComponent->scale[1];       

                globals.entities[i].uiComponent->uiNeedsUpdate = 0;
                
        }
    }
}

/**
 * @brief Movement system
 * Handles movement update on position,rotation & scale. (Atm only x-axis rotation.)
 * The model matrix that is used for rendering is NOT updated here, but in the modelSystem. ModelSystem uses 
 * the transform values that are updated here to update the model matrix. So this system needs to run before modelsystem.
 * Atm this system is more of a placeholder for movement logic, but will eventually be more complex. 
 * NOTE: Most movement logic is still done in the input function,but will eventually be moved here.
 */
void movementSystem(){
    // rotate model logic (temporary)
    float degrees = 15.5f * globals.delta_time;
    float radians = degrees * M_PI / 180.0f;

    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            
           if(globals.entities[i].transformComponent->active == 1){
                // Do movement logic here:

                // Example of movement logic: Rotate on y-axis on all entities that are not ui (temporary)
                if(globals.entities[i].uiComponent->active == 1){
                  //  printf("entity %d \n", i);
                    //printf("confirmed active ui\n");
                   /*  if(isPointInsideRect(globals.entities[i].uiComponent->boundingBox, (vec2){ globals.event.motion.x, globals.event.motion.y})){
                    globals.entities[i].transformComponent->scale[0] += 1.5f;
                    globals.entities[i].transformComponent->modelNeedsUpdate = 1; */
                     /*    printf("bb x %d ", globals.entities[i].uiComponent->boundingBox.x);
                        printf("bb y %d ", globals.entities[i].uiComponent->boundingBox.y);
                        printf("bb width %d ", globals.entities[i].uiComponent->boundingBox.width);
                        printf("bb height %d ", globals.entities[i].uiComponent->boundingBox.height); */
                        //globals.entities[i].transformComponent->rotation[1] = radians;
                        //globals.entities[i].transformComponent->modelNeedsUpdate = 1;
                   // }
                }
                // globals.entities[i].transformComponent->rotation[1] += radians; <- This is an example of acceleration.
           }
        }
    }
}

void hoverSystem(){
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].transformComponent->active == 1 && globals.entities[i].uiComponent->active == 1){

                    // Ui element pos is in ui-space-coords. We need to convert this to same as mouse coords, which is SDL-coords. Where x,y is TOP LEFT CORNER.
                    float halfWidth = (float)globals.entities[i].uiComponent->boundingBox.width / 2.0;
                    float halfHeight = (float)globals.entities[i].uiComponent->boundingBox.height / 2.0;
                    float uiViewHalfWidth = (float)globals.views.ui.rect.width / 2.0;
                    float uiViewHalfHeight = (float)globals.views.ui.rect.height / 2.0; // 100
                    // uiBoundingBoxToSDLCoordinates(&globals.entities[i]); -375.0, 75.0 -> 0, 400
                  
                    
                    float xLeftCornerSDL = globals.entities[i].uiComponent->boundingBox.x - halfWidth + uiViewHalfWidth; // 0
                    float yTopCornerSDL = (globals.entities[i].uiComponent->boundingBox.y - uiViewHalfHeight) * -1 + (float)globals.views.main.rect.height - halfHeight; // 25 - 100 = -75*-1 = 75 + 400 = 475 - 25 = 450
                  //  printf("xSDL %f \n", xLeftCornerSDL);
                  //  printf("ySDL %f \n", yTopCornerSDL);
                    

                    
                    
                    
                    Rectangle getRect;
                    getRect.y = yTopCornerSDL;
                    getRect.x = xLeftCornerSDL;
                    getRect.height = globals.entities[i].uiComponent->boundingBox.height;
                    getRect.width = globals.entities[i].uiComponent->boundingBox.width;
                    if(getRect.x >= globals.mouseXpos && getRect.x + getRect.width <= globals.mouseXpos && getRect.y >= globals.mouseYpos && getRect.y + getRect.height <= globals.mouseYpos){
                        printf("mouse is within bounding box\n");
                    }
                   // printf("ui_entity_pos_y %f \n", ui_entity_pos_y);
                   // printf("ui_entity_pos_x %f \n", ui_entity_pos_x);

                        // debug print
                       // printf("point x: %f y: %f \n",globals.mouseXpos,globals.mouseYpos);
                   /*      printf(
                            " x: %d y: %d width: %d height: %d \n",
                            getRect.x,getRect.y,K
                            getRect.width,
                            getRect.height); */
                    if(
                        globals.views.ui.isMousePointerWithin && 
                        isPointInsideRect(getRect, (vec2){ globals.mouseXpos, globals.mouseYpos})
                    ){
                    
                        
                        // Hover effect
                        globals.entities[i].materialComponent->useDiffuseMap = false;
                        globals.entities[i].materialComponent->ambient.g = 0.5f;
                        globals.entities[i].materialComponent->diffuse.g = 0.5f;
                        globals.entities[i].materialComponent->diffuse.a = 0.5f;
                    
                    } else {
                        globals.entities[i].materialComponent->useDiffuseMap = true;
                        globals.entities[i].materialComponent->ambient.g = 0.0f;
                        globals.entities[i].materialComponent->diffuse.g = 0.0f;
                        globals.entities[i].materialComponent->diffuse.a = 1.0f;
                    }
            }
        }
    } 
}

/**
 * @brief Model system
 * Handles model update.
 * Atm we are not handling everything about the model here, just the update of model matrix.
 * Movement is handled in the input function, but will eventually be moved here or to a separate movement-system. 
 * Perhaps the model matrix update will move there aswell or maybe be handled in some kind of transform hierarchy system, 
 * since groups & hierarchy of entities should be something we would need when we try to build more complex scenes.
 * NOTE: Atm this is more of a placeholder for model matrix update logic, since we only handle rotation on x-axis and not even handling scale change. 
 * So atm only works with rigid body transforms.
 */
void modelSystem(){
    // Look for transform needs update flag & update/recalc model matrix if needed.
     for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
           if(globals.entities[i].transformComponent->modelNeedsUpdate == 1) {
                    // create transformations
                    mat4x4 model;
                    mat4x4_identity(model);

                    // set model position
                    mat4x4_translate(model, globals.entities[i].transformComponent->position[0],globals.entities[i].transformComponent->position[1],globals.entities[i].transformComponent->position[2]);

                    // set model scale
                    mat4x4_scale_aniso(model, model, globals.entities[i].transformComponent->scale[0],globals.entities[i].transformComponent->scale[1],globals.entities[i].transformComponent->scale[2]);

                    // rotate model (atm only in x axis)
                    mat4x4 rotatedModel;
                    // The cast on model tells the compiler that you're aware of the 
                    // const requirement and that you're promising not to modify the model matrix.
                    mat4x4_rotate(rotatedModel, (const float (*)[4])model, 0.0f,1.0f,0.0f, globals.entities[i].transformComponent->rotation[1]);

                    // Copy the rotated model matrix to the transform component
                    memcpy(globals.entities[i].transformComponent->transform, rotatedModel, sizeof(mat4x4));

                    globals.entities[i].transformComponent->modelNeedsUpdate = 0;
           }
          }}
}

void update(){

    // Time 
    Uint32 ticks = SDL_GetTicks();

    // Cap the frame rate
    int time_to_wait = FRAME_TARGET_TIME - (ticks - last_frame_time);
    if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    // Set delta time in seconds
    globals.delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

    // Systems
    cameraSystem();
    uiSystem();
    hoverSystem();
    movementSystem();
    modelSystem();
}

void render(){

    // Clear the entire window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    // Native

    // TODO: find a way to avoid having to iterate over all entities twice, once for ui and once for 3d objects.

    // Render main view & 3d objects
   setViewportWithScissorAndClear(globals.views.main);
   setFontProjection(&globals.gpuFontData,globals.views.main);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1) {
                if(globals.entities[i].uiComponent->active != 1){
                
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
    }
    // Render ui scene & ui objects
    setViewportWithScissorAndClear(globals.views.ui);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1) {
                if(globals.entities[i].uiComponent->active == 1){
                        // render ui, could be overhead with switching viewports?. profile.
                        Color* diff = &globals.entities[i].materialComponent->diffuse;
                        Color* amb = &globals.entities[i].materialComponent->ambient;
                        Color* spec = &globals.entities[i].materialComponent->specular;
                        GLfloat shin = globals.entities[i].materialComponent->shininess;
                        GLuint diffMap = globals.entities[i].materialComponent->diffuseMap;
                        bool useDiffMap = globals.entities[i].materialComponent->useDiffuseMap;
                       
                        if(globals.drawBoundingBoxes){
                            if(globals.entities[i].tag == BOUNDING_BOX){
                                renderMesh(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,diff,amb,spec,shin,diffMap,globals.views.ui.camera,useDiffMap);
                            }
                        }else {
                            if(globals.entities[i].tag != BOUNDING_BOX){
                                renderMesh(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,diff,amb,spec,shin,diffMap,globals.views.ui.camera,useDiffMap);
                            }
                        }
                }
            }
        }
    }
    // render ui text
    setFontProjection(&globals.gpuFontData,globals.views.ui);
    glDisable(GL_DEPTH_TEST);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
                if(globals.entities[i].uiComponent->active == 1){
                    if(strlen(globals.entities[i].uiComponent->text) > 0){
                        printf("rendertext entity %d \n", i);
                        printf("transform position: %f %f \n", globals.entities[i].transformComponent->position[0], globals.entities[i].transformComponent->position[1]);
                        // convert transform position to viewport space
                        vec2 result;
                        convertUIcoordinateToWindowcoordinates(
                            globals.views.ui,
                            globals.entities[i].transformComponent,
                            height,
                            width,
                            result);
                          //  printf("result x %f \n", result[0]);
                          //  printf("result y %f \n", result[1]);
                            // align text center vertically
                            
                            result[1] -= (float)globals.characters[0].Size[1] / 4.0;
                          //  printf("globals.characters[0].Size %i \n", (float)globals.characters[0].Size[1] / 2.0);
                        renderText(
                            &globals.gpuFontData, 
                            globals.entities[i].uiComponent->text, 
                            result[0],result[1],
                            .5f,(Color){1.0f, 1.0f, 0.0f});
                    }
                }
        }
    }
    glEnable(GL_DEPTH_TEST);
   
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
   printf("OpenGL context created!\n");
}

/**
 * @brief Initialize the font
 * Load texture,setup mesh,setup material
 * After this the projection matrix for the font need to be set using setFontProjection.
 * And then you can render text using renderText.
 */
void initFont(){
    setupFontTextures("ARIAL.TTF",48);
    setupFontMesh(&globals.gpuFontData);
    setupFontMaterial(&globals.gpuFontData,width,height);
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

   // Assets
   initFont();
   TextureData textureData = loadTexture();
   GLuint diffuseTextureId = setupTexture(textureData);
  // ObjData objData = loadObjFile("truck.obj");

   // Main viewport objects (3d scene) x,y,z coords is a world space coordinate (not yet implemented).
// createObject(VIEWPORT_MAIN,green,diffuseTextureId,&objData);
// createObject(VIEWPORT_UI,red,diffuseTextureId,&objData);
   //createCube(VIEWPORT_MAIN,red,diffuseTextureId);
   createLight(VIEWPORT_MAIN,green,diffuseTextureId);
   // createCube(VIEWPORT_UI,red,diffuseTextureId);
  // createCube(VIEWPORT_MAIN,green,diffuseTextureId);


   // UI scene objects creation (2d scene) x,y coords where x = 0 is left and y = 0 is top and x,y is pixel positions. 
   // Scale is in pixels, 100.0f is 100 pixels etc.
   // z position will be z-depth, much like in DOM in web.
   // TODO: implement rotation, it is atm not affecting. 
  // createRectangle(VIEWPORT_UI,red,diffuseTextureId, (vec3){350.0f, 0.0f, 0.0f}, (vec3){100.0f, 100.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f});
   createRectangle(VIEWPORT_UI,red,diffuseTextureId, (vec3){600.0f, 0.0f, 0.0f}, (vec3){100.0f, 100.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f});
 //  createRectangle(VIEWPORT_UI,red,diffuseTextureId, (vec3){50.0f, 5.0f, 0.0f}, (vec3){35.0f, 50.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f});
  // createRectangle(VIEWPORT_UI,red,diffuseTextureId, (vec3){300.0f, 120.0f, 0.0f}, (vec3){100.0f, 40.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f});
  
   
   

}

int main(int argc, char **argv) {
    
    // Initialize the random number generator
    srand((unsigned int)time(NULL));
    
    initProgram();
   
    initOpenGLWindow();
    
    // Camera setup
    Camera* uiCamera = initCamera();
    uiCamera->isOrthographic = 1;
    uiCamera->aspectRatio = globals.views.ui.rect.width / globals.views.ui.rect.height;
    uiCamera->left = -800.0f/2.0f;
    uiCamera->right = 800.0f/2.0f;
    uiCamera->bottom = -200.0f/2.0f;
    uiCamera->top = 200.0f/2.0f;
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

/**
 * @brief Create a mesh
 * Main function to create a mesh. 
 *  - vertex data
 *  - index data
 *  - transform data
 *  - material data
*/
void createMesh(
    GLfloat* verts,
    GLuint num_of_vertex, 
    GLuint* indices, // atm plug in some dummy-data if not used.
    GLuint numIndicies, // atm just set to 0 if not used.
    vec3 position,
    vec3 scale,
    vec3 rotation,
    Material* material,
    int ui,
    GLenum drawMode,
    VertexDataType vertexDataType,
    Entity* entity
    ){
    
    entity->meshComponent->active = 1;

    // vertex data
    entity->meshComponent->vertices = (Vertex*)malloc(num_of_vertex * sizeof(Vertex));
     if (entity->meshComponent->vertices == NULL) {
        printf("Failed to allocate memory for vertices\n");
        exit(1);
    }
    
    int stride = 8;
    int vertexIndex = 0;

    // We have three types of vertex data input:
    // - one with color & indices VERT_COLOR_INDICIES
    // - one with color & no indicies VERT_COLOR
    // - one with no color & no indicies VERT
    if(numIndicies == 0 && vertexDataType == VERTS_ONEUV) {

        // vertices + texcoords but no indices and no color
        stride = 8;
        for(int i = 0; i < num_of_vertex * stride; i+=stride) {
            entity->meshComponent->vertices[vertexIndex].position[0] = verts[i];
            entity->meshComponent->vertices[vertexIndex].position[1] = verts[i + 1];
            entity->meshComponent->vertices[vertexIndex].position[2] = verts[i + 2];

            entity->meshComponent->vertices[vertexIndex].color[0] = verts[i + 3];
            entity->meshComponent->vertices[vertexIndex].color[1] = verts[i + 4];
            entity->meshComponent->vertices[vertexIndex].color[2] = verts[i + 5]; 

            entity->meshComponent->vertices[vertexIndex].texcoord[0] = verts[i + 6];
            entity->meshComponent->vertices[vertexIndex].texcoord[1] = verts[i + 7];
            vertexIndex++;
        } 

    }else if(vertexDataType == VERTS_COLOR_ONEUV_INDICIES){

        // indexed data with color
        for(int i = 0; i < num_of_vertex * stride; i+=stride) {
            entity->meshComponent->vertices[vertexIndex].position[0] = verts[i];
            entity->meshComponent->vertices[vertexIndex].position[1] = verts[i + 1];
            entity->meshComponent->vertices[vertexIndex].position[2] = verts[i + 2];
            entity->meshComponent->vertices[vertexIndex].color[0] = verts[i + 3]; 
            entity->meshComponent->vertices[vertexIndex].color[1] = verts[i + 4];
            entity->meshComponent->vertices[vertexIndex].color[2] = verts[i + 5];
            entity->meshComponent->vertices[vertexIndex].texcoord[0] = verts[i + 6];
            entity->meshComponent->vertices[vertexIndex].texcoord[1] = verts[i + 7];
            vertexIndex++;
        }
    }else if(vertexDataType == VERTS_COLOR_ONEUV){
         // not indexed data with color
        for(int i = 0; i < num_of_vertex * stride; i+=stride) {
            entity->meshComponent->vertices[vertexIndex].position[0] = verts[i];
            entity->meshComponent->vertices[vertexIndex].position[1] = verts[i + 1];
            entity->meshComponent->vertices[vertexIndex].position[2] = verts[i + 2];
            entity->meshComponent->vertices[vertexIndex].color[0] = verts[i + 3]; 
            entity->meshComponent->vertices[vertexIndex].color[1] = verts[i + 4];
            entity->meshComponent->vertices[vertexIndex].color[2] = verts[i + 5];
            entity->meshComponent->vertices[vertexIndex].texcoord[0] = verts[i + 6];
            entity->meshComponent->vertices[vertexIndex].texcoord[1] = verts[i + 7];
            vertexIndex++;
        }
    }else {
        printf("UNSUPPORTED vertexData");
        exit(1);
    }
    
    entity->meshComponent->vertexCount = num_of_vertex;
    entity->meshComponent->gpuData->vertexCount = num_of_vertex;

    // index data
    entity->meshComponent->indices = (GLuint*)malloc(numIndicies * sizeof(GLuint));
    if(entity->meshComponent->indices == NULL) {
        printf("Failed to allocate memory for indices\n");
        exit(1);
    }
    for(int i = 0; i < numIndicies; i++) {
        entity->meshComponent->indices[i] = indices[i];
    }
    entity->meshComponent->indexCount = numIndicies;

    if(numIndicies == 0){
        printf("not using indicies\n");
    }

    printf("position %f %f %f\n", position[0], position[1], position[2]);

    // transform data
    entity->transformComponent->active = 1;
    entity->transformComponent->position[0] = position[0];
    entity->transformComponent->position[1] = position[1];
    entity->transformComponent->position[2] = position[2];
    entity->transformComponent->scale[0] = scale[0];
    entity->transformComponent->scale[1] = scale[1];
    entity->transformComponent->scale[2] = scale[2];
    entity->transformComponent->rotation[0] = rotation[0];
    entity->transformComponent->rotation[1] = rotation[1];
    entity->transformComponent->rotation[2] = rotation[2];
    entity->transformComponent->modelNeedsUpdate = 1;

    // material data
    entity->materialComponent->active = 1;
    entity->materialComponent->ambient = material->ambient;
    entity->materialComponent->diffuse = material->diffuse;
    entity->materialComponent->specular = material->specular;
    entity->materialComponent->shininess = material->shininess;
    entity->materialComponent->diffuseMap = material->diffuseMap;
    entity->meshComponent->gpuData->drawMode = drawMode;

    setupMesh(  entity->meshComponent->vertices, 
                entity->meshComponent->vertexCount, 
                entity->meshComponent->indices, 
                entity->meshComponent->indexCount,
                entity->meshComponent->gpuData
                );
    
    // Print mesh setup data for debugging
    // gpuData vao:
    printf("vao: %d\n", entity->meshComponent->gpuData->VAO);
    printf("vbo: %d\n", entity->meshComponent->gpuData->VBO);
    printf("ebo: %d\n", entity->meshComponent->gpuData->EBO);


    setupMaterial( entity->meshComponent->gpuData );
}


void createLight(int ui,Color diffuse,GLuint diffuseTextureId){
    // vertex data
    GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };
    // index data NOT USED ATM
    GLuint indices[] = {
        // front and back
        0, 3, 2,
        2, 1, 0,
        4, 5, 6,
        6, 7 ,4,
        // left and right
        11, 8, 9,
        9, 10, 11,
        12, 13, 14,
        14, 15, 12,
        // bottom and top
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };
    // transform
    vec3 position = {0.0f, 0.0f, 0.0f};
    vec3 scale =    {1.0f, 1.0f, 1.0f};
    vec3 rotation = {0.0f, 0.0f, 0.0f};

    //material
    Color ambient = {1.0f, 1.0f, 0.0f, 1.0f};
    //Color diffuse = {0.0f, 0.0f, 1.0f, 1.0f}; not used, comes from attribute data
    Color specular = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat shininess = 32.0f;
    Material material = {ambient, diffuse, specular, shininess, diffuseTextureId, true};

    Entity* entity = addEntity(MODEL);
    if(ui == 1){
        entity->uiComponent->active = 1;
        entity->uiComponent->boundingBox = (Rectangle){0,0,100,100};
    }

    createMesh(vertices,36,indices,0,position,scale,rotation,&material,ui,GL_TRIANGLES,VERTS_ONEUV,entity);
}
/**
 * @brief Create a object. Used together with obj-load/parse. Expects data from obj-parser to be of type ObjData.
 * Create a object mesh
 * @param ui - 1 for ui, 0 for 3d scene
 * @param diffuse - color of the rectangle
*/
void createObject(int ui,Color diffuse,GLuint diffuseTextureId,ObjData* obj){
   // vertex data
    int stride = 8;
    GLfloat vertices[(obj->num_of_vertices)*stride];
    for(int i = 0; i < obj->num_of_vertices; i++){
            vertices[i * stride + 0] = obj->vertexData[i].position[0];
            vertices[i * stride + 1] = obj->vertexData[i].position[1];
            vertices[i * stride + 2] = obj->vertexData[i].position[2];
            vertices[i * stride + 3] = obj->vertexData[i].color[0];
            vertices[i * stride + 4] = obj->vertexData[i].color[1];
            vertices[i * stride + 5] = obj->vertexData[i].color[2];
            vertices[i * stride + 6] = obj->vertexData[i].texcoord[0];
            vertices[i * stride + 7] = obj->vertexData[i].texcoord[1];
    }  
   
    // NOT USED
    GLuint indices[] = {
        0, 1, 2,  // first triangle
        1, 2, 3   // second triangle
    }; 

    // transform
    vec3 position = {0.0f, 0.0f, 0.0f};
    vec3 scale = {1.0f, 1.0f, 1.0f};
    vec3 rotation = {0.0f, 0.0f, 0.0f};

    //material
    Color ambient = {1.0f, 0.1f, 0.1f, 1.0f};
    Color specular = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat shininess = 32.0f;
    Material material = {ambient, diffuse, specular, shininess, diffuseTextureId, true};

    Entity* entity = addEntity(MODEL);
    if(ui == 1){
        entity->uiComponent->active = 1;
    }

    createMesh(vertices,obj->num_of_vertices,indices,0,position,scale,rotation,&material,ui,GL_TRIANGLES,VERTS_COLOR_ONEUV,entity);
}

/**
 * @brief Create a triangle
 * Create a triangle mesh
 * @param ui - 1 for ui, 0 for 3d scene
 * @param diffuse - color of the triangle
 * TODO: does not work with wasm atm. indices? GLtype?
*/
void createTriangle(int ui,Color diffuse){
    // vertex data
    GLfloat verts[] = {
         0.0f,  0.5f, 0.0f,  // top
        -0.5f, -0.5f, 0.0f,  // bottom left
         0.5f, -0.5f, 0.0f   // bottom right
    };
    // index data
    GLuint indices[] = {
        0, 1, 2,  // first triangle
        1, 2, 3   // second triangle
    }; 
    // transform
    vec3 position = {0.0f, 0.0f, 0.0f};
    vec3 scale = {1.0f, 1.0f, 1.0f};
    vec3 rotation = {0.0f, 0.0f, 0.0f};

    //material
    Color ambient = {0.1f, 0.1f, 0.1f, 1.0f};
    //Color diffuse = diffuseColor;
    Color specular = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat shininess = 32.0f;
    Material material = {ambient, diffuse, specular, shininess};

    Entity* entity = addEntity(MODEL);
    if(ui == 1){
        entity->uiComponent->active = 1;
    }

    createMesh(verts,3,indices,6,position,scale,rotation,&material,ui,GL_TRIANGLES,VERTS_ONEUV,entity);
}
/**
 * @brief Create a rectangle
 * Create a rectangle mesh
 * @param ui - 1 for ui, 0 for 3d scene
 * @param diffuse - color of the rectangle
*/
void createRectangle(int ui,Color diffuse,GLuint diffuseTextureId,vec3 position,vec3 scale,vec3 rotation){
    // vertex data
    GLfloat vertices[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f, // top right
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f  // top left  
    };
    // index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    //material
    Color ambient = {0.1f, 0.1f, 0.1f, 1.0f};
   // Color diffuse = {0.0f, 0.0f, 1.0f, 1.0f};
    Color specular = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat shininess = 32.0f;
    Material material = {ambient, diffuse, specular, shininess, diffuseTextureId,true};

    Entity* entity = addEntity(MODEL);
    if(ui == 1){
        entity->uiComponent->active = 1;
        entity->uiComponent->boundingBox = (Rectangle){0,0,100,100};
        entity->uiComponent->text = "Test";
        entity->uiComponent->uiNeedsUpdate =1;

    }

    createMesh(vertices,4,indices,6,position,scale,rotation,&material,ui,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity);

    // Bounding box, reuses the vertices from the rectangle
    Material boundingBoxMaterial = {{0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, specular, shininess, diffuseTextureId, false};
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    if(ui == 1){
        boundingBoxEntity->uiComponent->active = 1;
        boundingBoxEntity->uiComponent->uiNeedsUpdate = 1;

    } 
    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&boundingBoxMaterial,ui,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity);
    
}




/**
 * @brief Create a Cube
 * Create a Cube mesh
 * @param ui - 1 for ui, 0 for 3d scene
 * @param diffuse - color of the cube
*/
void createCube(int ui,Color diffuse,GLuint diffuseTextureId){
    // vertex data
    GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f
};
    // index data NOT USED ATM
    GLuint indices[] = {
        // front and back
        0, 3, 2,
        2, 1, 0,
        4, 5, 6,
        6, 7 ,4,
        // left and right
        11, 8, 9,
        9, 10, 11,
        12, 13, 14,
        14, 15, 12,
        // bottom and top
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };
    // transform
    vec3 position = {0.0f, 0.0f, 0.0f};
    vec3 scale = {1.0f, 1.0f, 1.0f};
    vec3 rotation = {0.0f, 0.0f, 0.0f};

    //material
    Color ambient = {0.1f, 0.1f, 0.1f, 1.0f};
   // Color diffuse = {0.0f, 0.0f, 1.0f, 1.0f};
    Color specular = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat shininess = 32.0f;
    Material material = {ambient, diffuse, specular, shininess, diffuseTextureId,true };

    Entity* entity = addEntity(MODEL);
    if(ui == 1){
        entity->uiComponent->active = 1;
    }

    createMesh(vertices,36,indices,0,position,scale,rotation,&material,ui,GL_TRIANGLES,VERTS_ONEUV,entity);
}

/**
 * @brief Create a camera
 */
Camera* initCamera() {
    Camera* camera = (Camera*)malloc(sizeof(Camera));
    if (camera == NULL) {
        // Handle memory allocation failure
        return NULL;
    }
    vec3 position = {0.0f, 0.0f, 3.0f};
    vec3 front = {0.0f, 0.0f, -1.0f};
    vec3 up = {0.0f, 1.0f, 0.0f};
    vec3 target = {0.0f, 0.0f, 0.0f};

    // Initialize the Camera fields
    memcpy(camera->position, position, sizeof(vec3));
    memcpy(camera->front, front, sizeof(vec3));
    memcpy(camera->up, up, sizeof(vec3));
    memcpy(camera->target, target, sizeof(vec3));
    // Initialize other fields as needed
    camera->speed = 0.1f;
    camera->viewMatrixNeedsUpdate = 1;
    camera->projectionMatrixNeedsUpdate = 1;
    camera->fov = 45.0f;
    camera->near = 0.1f;
    camera->far = 100.0f;
    camera->aspectRatio = 800.0f / 600.0f;
    camera->left = -1.0f;
    camera->right = 1.0f;
    camera->bottom = -1.0f;
    camera->top = 1.0f;
    camera->isOrthographic = 0;

    return camera;
}

// -----------------------------------------------------
// ASSETS
// -----------------------------------------------------
/**
 * @brief Load a texture
 * Load a texture from file
*/
TextureData loadTexture(){
    int width, height, nrChannels;
    unsigned char *data = loadImage("container.jpg", &width, &height, &nrChannels); 
    if(data == NULL) {
        printf(TEXT_COLOR_ERROR "Failed to load texture\n" TEXT_COLOR_RESET);
        exit(1);
    }
    TextureData textureData = {data, width, height, nrChannels};
    return textureData;
}


    
