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
        .ui={{0, 0, 800, 200}, {0.0f, 0.0f, 0.0f, 1.0f}, SPLIT_HORIZONTAL, NULL,NULL,false},
        .main={{0, 200, 800, 400}, {1.0f, 0.0f, 0.0f, 1.0f}, SPLIT_DEFAULT, &globals.views.ui,NULL,false},
        .full={{0, 0, 800, 600}, {0.0f, 0.0f, 1.0f, 1.0f}, SPLIT_DEFAULT, NULL,NULL,false},
    },
    .firstMouse=1,
    .mouseXpos=0.0f,
    .mouseYpos=0.0f,
};

// Colors
Color blue = {0.0f, 0.0f, 1.0f, 1.0f};
Color red = {1.0f, 0.0f, 0.0f, 1.0f};
Color green = {0.0f, 1.0f, 0.0f, 1.0f};

// Window dimensions
static const int width = 800;
static const int height = 600;

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

void initializeUIComponent(UIComponent* uiComponent){
    uiComponent->active = 0;
    uiComponent->hovered = 0;
    uiComponent->clicked = 0;
    uiComponent->boundingBox.x = 0;
    uiComponent->boundingBox.y = 0;
    uiComponent->boundingBox.width = 0;
    uiComponent->boundingBox.height = 0;
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
        "Hello, SDL2", 
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

void setViewportWithScissor(View view) {
    // Set the viewport
    glViewport(view.rect.x, view.rect.y, view.rect.width, view.rect.height);

    // Set the scissor box
    glScissor(view.rect.x, view.rect.y, view.rect.width, view.rect.height);
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

void recalculateViewports(int w, int h){
    // Recalculate viewports
  if(globals.views.main.childView != NULL){
        if(globals.views.main.childView->splitDirection == SPLIT_HORIZONTAL){

            // Calc new percentage height, using old main height.
            float percentageChildHeight = (float)globals.views.main.childView->rect.height / ((float)globals.views.main.rect.height + (float)globals.views.main.childView->rect.height);
            printf("percentageChildHeight: %f\n", percentageChildHeight);

            // Update childView height & the new width.
            globals.views.main.childView->rect.height = h * percentageChildHeight;
            printf("new childView height: %d\n", globals.views.main.childView->rect.height);
            globals.views.main.childView->rect.width = w;

            // Update main height
            globals.views.main.rect.height = h - globals.views.main.childView->rect.height;
            globals.views.main.rect.width = w;

            // Update main y position
            globals.views.main.rect.y = globals.views.main.childView->rect.height;
            
            // TODO: here we need to update the childView y position,x pos & main x pos aswell. Atm they are not handled.

        }
        if(globals.views.main.childView->splitDirection == SPLIT_VERTICAL){
            // Calc new percentage width, using old main width.
            float percentageChildWidth = globals.views.main.childView->rect.width / globals.views.main.rect.width;

            // Update childView height & the new width.
            globals.views.main.childView->rect.width = w * percentageChildWidth;
            globals.views.main.childView->rect.height = h;

            // Update main height
            globals.views.main.rect.height = h;
            globals.views.main.rect.width = w - globals.views.main.childView->rect.width;
        }
    }
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

            
           
            // If mouse pos is within the main view
            // view is not a rectangle with SDL-coords, we need to flip the y-value.
            Rectangle convertedMainViewRectangle = convertViewRectangleToSDLCoordinates(globals.views.main,height);
     
            if(isPointInsideRect(convertedMainViewRectangle, (vec2){xpos, ypos})){ 
                
               // printf("Mouse is within main view\n ");
                // Update the camera front vector
                calcCameraFront(globals.views.main.camera,xpos, ypos);
                // set view matrix needs update flag (so that we recalculate the view matrix with the new front vector)
                globals.views.main.camera->viewMatrixNeedsUpdate = 1;
            }

            // If mouse pos is within the ui view
            // view is not a rectangle with SDL-coords, we need to flip the y-value.
            Rectangle convertedUIViewRectangle = convertViewRectangleToSDLCoordinates(globals.views.ui,height);
     
            if(isPointInsideRect(convertedUIViewRectangle, (vec2){xpos, ypos})){
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
        if(globals.entities[i].alive == 1 && globals.entities[i].uiComponent->active) {
            // Do ui logic here
            if(globals.entities[i].transformComponent->modelNeedsUpdate == 1){
                
                
                // Position element in "UI" space, where 0,0 is the center of the ui-viewport screen. We will position the element to top left corner of the screen.
                float ui_viewport_half_width = (float)globals.views.main.rect.width / 2;
                float ui_viewport_half_height = (float)globals.views.main.rect.height / 2;
                float entity_half_scale_x = globals.entities[i].transformComponent->scale[0] / 2;
                float entity_half_scale_y = globals.entities[i].transformComponent->scale[1] / 2;
                globals.entities[i].transformComponent->position[0] = globals.entities[i].transformComponent->position[0] + (float)globals.views.main.rect.x - ui_viewport_half_width + entity_half_scale_x;
                globals.entities[i].transformComponent->position[1] = globals.entities[i].transformComponent->position[1] + (float)globals.views.main.rect.y - ui_viewport_half_height + entity_half_scale_y;
                
                globals.entities[i].uiComponent->boundingBox.x = globals.entities[i].transformComponent->position[0] + ui_viewport_half_width - entity_half_scale_x;
                globals.entities[i].uiComponent->boundingBox.y = globals.entities[i].transformComponent->position[1] + ui_viewport_half_height - entity_half_scale_y;
                globals.entities[i].uiComponent->boundingBox.width = globals.entities[i].transformComponent->scale[0] + globals.entities[i].transformComponent->position[0] + ui_viewport_half_width - entity_half_scale_x;
                globals.entities[i].uiComponent->boundingBox.height = globals.entities[i].transformComponent->scale[1] + globals.entities[i].transformComponent->position[1] + ui_viewport_half_height - entity_half_scale_y;

               // printf("ui viewport x & y pos: %f, %f\n", (float)globals.views.main.rect.x, (float)globals.views.main.rect.y);
                //printf("pos x & y: %f, %f\n", globals.entities[i].transformComponent->position[0], globals.entities[i].transformComponent->position[1]);
                printf("bb x %d \n", globals.entities[i].uiComponent->boundingBox.x);
                printf("bb y %d \n", globals.entities[i].uiComponent->boundingBox.y);
                printf("bb width %d \n", globals.entities[i].uiComponent->boundingBox.width);
                printf("bb height %d \n", globals.entities[i].uiComponent->boundingBox.height);
                printf("----------------------------------------\n");
              


                
            }

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
    // Check if mouse is inside ui view
    // if so, check if mouse is inside any ui elements
    // if so, set hover effect on that element
    for(int i = 0; i < MAX_ENTITIES; i++) {
                    if(globals.entities[i].alive == 1) {
                        if(globals.entities[i].transformComponent->active == 1 && globals.entities[i].uiComponent->active == 1){
                            // Do movement logic here:

                            // Example of movement logic: Rotate on y-axis on all entities that are not ui (temporary)
                           
                                /*   printf("bb x %d \n", globals.entities[i].uiComponent->boundingBox.x);
                                    printf("bb y %d \n", globals.entities[i].uiComponent->boundingBox.y);
                                    printf("bb width %d \n", globals.entities[i].uiComponent->boundingBox.width);
                                    printf("bb height %d \n", globals.entities[i].uiComponent->boundingBox.height); */
                                    float ndcX = (float)globals.entities[i].uiComponent->boundingBox.x; /// (float)width;
                                    float ndcY = (float)globals.entities[i].uiComponent->boundingBox.y; /// (float)height;
                                    float ndcWidth = (float)globals.entities[i].uiComponent->boundingBox.width; /// (float)width;
                                    float ndcHeight = (float)globals.entities[i].uiComponent->boundingBox.height;/// (float)height;

                                    float ui_viewport_half_width = (float)globals.views.main.rect.width / 2;
                                    float ui_viewport_half_height = (float)globals.views.main.rect.height / 2;
                                
                                    printf("ndcX %f \n", ndcX-ui_viewport_half_width);
                                    printf("ndcY %f \n", ndcY+ui_viewport_half_height);
                                    printf("ndcWidth %f \n", ndcWidth);
                                    printf("ndcHeight %f \n", ndcHeight);
                                    printf("----------------------------------------\n");
                                    Rectangle ndcRect = {ndcX, ndcY, ndcWidth, ndcHeight};
                                if(globals.views.ui.isMousePointerWithin && isPointInsideRect(ndcRect, (vec2){ globals.mouseXpos, globals.mouseYpos})){
                                //globals.entities[i].transformComponent->scale[0] += 0.05f;
                                    //globals.entities[i].transformComponent->rotation[1] = radians;
                                    globals.entities[i].materialComponent->useDiffuseMap = false;
                                    globals.entities[i].materialComponent->ambient.g = 0.5f;
                                    globals.entities[i].materialComponent->diffuse.g = 0.5f;
                                    globals.entities[i].materialComponent->diffuse.a = 0.5f;
                                //globals.entities[i].transformComponent->modelNeedsUpdate = 1;
                                    
                                    //globals.entities[i].transformComponent->modelNeedsUpdate = 1;
                                } else {
                                    globals.entities[i].materialComponent->useDiffuseMap = true;
                                    globals.entities[i].materialComponent->ambient.g = 0.0f;
                                    globals.entities[i].materialComponent->diffuse.g = 0.0f;
                                    globals.entities[i].materialComponent->diffuse.a = 1.0f;
                                }
                            
                            // globals.entities[i].transformComponent->rotation[1] += radians; <- This is an example of acceleration.
                        }
                    }
                }
                // TODO: 
                // 1. set a flag somewhere so that check that flag later in update loop to see if we are inside ui view.
                // 2. make a system, like uiSystem where we iterate over all enties with uiComponent and do stuff with them, 
                // maybe if type is button we check if mouse is inside using isPointInsideRect and do hover effect?.
                // 3. text rendering on button?
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
                renderMesh(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,diff,amb,spec,shin,diffMap,globals.views.main.camera);
            }
        }
    }
    #else

    // Native
    // Render 3d scene 
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1) {
                if(globals.entities[i].uiComponent->active == 1){
                    // render ui, could be overhead with switching viewports?. profile.
                    setViewportWithScissor(globals.views.ui);
                    Color* diff = &globals.entities[i].materialComponent->diffuse;
                    Color* amb = &globals.entities[i].materialComponent->ambient;
                    Color* spec = &globals.entities[i].materialComponent->specular;
                    GLfloat shin = globals.entities[i].materialComponent->shininess;
                    GLuint diffMap = globals.entities[i].materialComponent->diffuseMap;
                    bool useDiffMap = globals.entities[i].materialComponent->useDiffuseMap;
                    renderMesh(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,diff,amb,spec,shin,diffMap,globals.views.ui.camera,useDiffMap);
                }else {
                    setViewportWithScissor(globals.views.main);
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
   TextureData textureData = loadTexture();
   GLuint diffuseTextureId = setupTexture(textureData);
   ObjData objData = loadObjFile("truck.obj");

   // Main viewport objects (3d scene) x,y,z coords is a world space coordinate (not yet implemented).
 //createObject(VIEWPORT_MAIN,green,diffuseTextureId,&objData);
// createObject(VIEWPORT_UI,red,diffuseTextureId,&objData);
   //createCube(VIEWPORT_MAIN,red,diffuseTextureId);
   createLight(VIEWPORT_MAIN,green,diffuseTextureId);
   // createCube(VIEWPORT_UI,red,diffuseTextureId);
  // createCube(VIEWPORT_MAIN,green,diffuseTextureId);
    //exit(1);
    
   // createRectangle(VIEWPORT_MAIN,blue,diffuseTextureId);

   // UI scene objects creation (2d scene) x,y coords where x = 0 is left and y = 0 is top and x,y is pixel coords.
   // TODO: implement point attributes (which is in ndc coords), to pixel coords. z position will be z-depth, much like in DOM in web.
   createRectangle(VIEWPORT_UI,red,diffuseTextureId, (vec3){0.0f, 0.0f, 0.0f}, (vec3){100.0f, 100.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f});

}

int main(int argc, char **argv) {
    
    // Initialize the random number generator
    srand((unsigned int)time(NULL));
    
    initProgram();
   
    initOpenGLWindow();
    
    
    
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
    

    setupMesh(  entity->meshComponent->vertices, 
                entity->meshComponent->vertexCount, 
                entity->meshComponent->indices, 
                entity->meshComponent->indexCount,
                entity->meshComponent->gpuData
                );
    
    
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
        entity->uiComponent->boundingBox = (Rectangle){0,400,50,50};
        // Create a button component with an initial position in ndc space coords. 1.0f is one pixel.
    }

    createMesh(vertices,4,indices,6,position,scale,rotation,&material,ui,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity);
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
