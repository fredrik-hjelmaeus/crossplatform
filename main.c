#include <stdio.h>
#include <time.h>
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
#include "opengl_types.h"
#include "types.h"
#include "globals.h"
#include "linmath.h"
#include "utils.h"
#include "opengl.h"

// Stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Prototypes
int ui_createRectangle(Material material,vec3 position,vec3 scale,vec3 rotation);
void createCube(Material material,vec3 position,vec3 scale,vec3 rotation);
void createObject(ObjData* obj,vec3 position,vec3 scale,vec3 rotation);
void createLight(Material material,vec3 position,vec3 scale,vec3 rotation,vec3 direction,LightType type);
void modelSystem();
void uiSystem();
void uiInputSystem();
void onButtonClick();
void onTextInputChange();
void ui_createButton(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,ButtonCallback onClick);
void ui_createTextInput(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,OnChangeCallback onChange);
void createPlane(Material material,vec3 position,vec3 scale,vec3 rotation);
void createLine(vec3 position, vec3 endPosition,Entity* entity);
void createPoints(GLfloat* positions,int numPoints, Entity* entity);
void moveCursor(float x);
ClosestLetter findCharacterUnderCursor();
Entity* addEntity(enum Tag tag);
void deleteEntity(Entity* entity);
ClosestLetter getClosestLetterInText(UIComponent* uiComponent, float mouseX);
int addMaterial(Material material);
Vector2 convertSDLToUI(float x, float y);
Vector2 convertUIToSDL(float x, float y);
Material* getMaterial(int index);
void createMesh(
    GLfloat* verts,
    GLuint num_of_vertex, 
    GLuint* indices, // atm plug in some dummy-data if not used.
    GLuint numIndicies, // atm just set to 0 if not used.
    vec3 position,
    vec3 scale,
    vec3 rotation,
    Material* material,
    GLenum drawMode,
    VertexDataType vertexDataType,
    Entity* entity,
    bool saveMaterial
    );

Camera* initCamera();


#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

//------------------------------------------------------
// Global variables / Initialization
//------------------------------------------------------

// Window dimensions
static const int width = 800;  // If these change, the views defaults should be changed aswell.
static const int height = 600; // If these change, the views defaults should be changed aswell.

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
    .firstMouse=1,
    .mouseXpos=0.0f,
    .mouseYpos=0.0f,
    .mouseLeftButtonPressed=false,
    .drawBoundingBoxes=false,
    .render=true,
    .gpuFontData={0,0,0,0,0,0,GL_TRIANGLES},
    .unitScale=100.0f, // 1 unit = 1 cm 
    .culling=false,
    .drawCallsCounter=0,
    .debugDrawCalls=false,
    //.assetArena=NULL,
    .materials=NULL,
    .materialsCount=0,
    .materialsCapacity=15,
    .objDataCapacity=10,
    .lights={0},
    .lightsCount=0,
    .focusedEntityId=-1,
    .cursorEntityId=-1,
    .charScale=0.5f
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
//  ECS
//------------------------------------------------------
// Every entity get's all components attached to it and memory is allocated for all components.

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
    meshComponent->gpuData = (GpuData*)malloc(sizeof(GpuData)); // TODO: replace with arena alloc?
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
    materialComponent->diffuseMapOpacity = 0.0f;
    materialComponent->diffuseMap = 0;
    materialComponent->specularMap = 0;
    materialComponent->materialIndex = -1;
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
    uiComponent->boundingBoxEntityId = -1;
    uiComponent->onClick = NULL;
    uiComponent->onChange = NULL;
    uiComponent->type = UITYPE_NONE;
}

void initializeLightComponent(LightComponent* lightComponent){
    lightComponent->active = 0;
    lightComponent->direction[0] = 0.0f;
    lightComponent->direction[1] = 0.0f;
    lightComponent->direction[2] = 0.0f;
    lightComponent->intensity = 0.0f;
    lightComponent->ambient.r = 0.0f;
    lightComponent->ambient.g = 0.0f;
    lightComponent->ambient.b = 0.0f;
    lightComponent->ambient.a = 1.0f;
    lightComponent->diffuse.r = 0.0f;
    lightComponent->diffuse.g = 0.0f;
    lightComponent->diffuse.b = 0.0f;
    lightComponent->diffuse.a = 1.0f;
    lightComponent->specular.r = 0.0f;
    lightComponent->specular.g = 0.0f;
    lightComponent->specular.b = 0.0f;
    lightComponent->specular.a = 1.0f;
}

void initializeLineComponent(LineComponent* lineComponent){
    lineComponent->active = 0;
    lineComponent->gpuData = (GpuData*)malloc(sizeof(GpuData)); // TODO: replace with arena alloc?
    if(lineComponent->gpuData == NULL) {
        printf("Failed to allocate memory for gpuData\n");
        exit(1);
    }
    // Initialize GpuData
    lineComponent->gpuData->VAO = 0;
    lineComponent->gpuData->VBO = 0;
    lineComponent->gpuData->EBO = 0;
    lineComponent->gpuData->drawMode = GL_LINES; // Default draw mode
    lineComponent->gpuData->numIndicies = 0;
    lineComponent->color.r = 0.0f;
    lineComponent->color.g = 0.0f;
    lineComponent->color.b = 0.0f;
    lineComponent->color.a = 1.0f;
    lineComponent->start[0] = 0.0f;
    lineComponent->start[1] = 0.0f;
    lineComponent->start[2] = 0.0f;
    lineComponent->end[0] = 0.0f;
    lineComponent->end[1] = 0.0f;
    lineComponent->end[2] = 0.0f;
}

void initializePointComponent(PointComponent* pointComponent){
    pointComponent->active = 0;
    pointComponent->gpuData = (GpuData*)malloc(sizeof(GpuData)); // TODO: replace with arena alloc?
    if(pointComponent->gpuData == NULL) {
        printf("Failed to allocate memory for gpuData\n");
        exit(1);
    }
    pointComponent->points = NULL;
    pointComponent->color.r = 0.0f;
    pointComponent->color.g = 0.0f;
    pointComponent->color.b = 0.0f;
    pointComponent->color.a = 1.0f;
    pointComponent->pointSize = 1.0f;
}


/**
 * @brief Initialize the ECS
 * Allocates memory pools for the components and entities.
 * This means that for every new component type we add, entity size grows.
 * This is a tradeoff between memory and performance. 
 * Having the components already allocated on startup is faster, but uses more memory.
 * If component size grows too large, we might consider allocating them on the fly or have a memory pool for each component type?
*/
void initECS(){
    // Allocate memory for MAX_ENTITIES Components structs
    TransformComponent* transformComponents = allocateComponentMemory(sizeof(TransformComponent), "transform");
    GroupComponent* groupComponents = allocateComponentMemory(sizeof(GroupComponent), "group");
    MeshComponent* meshComponents = allocateComponentMemory(sizeof(MeshComponent), "mesh");
    MaterialComponent* materialComponents = allocateComponentMemory(sizeof(MaterialComponent), "material");
    UIComponent* uiComponents = allocateComponentMemory(sizeof(UIComponent), "ui");
    LightComponent* lightComponents = allocateComponentMemory(sizeof(LightComponent), "light");
    LineComponent* lineComponents = allocateComponentMemory(sizeof(LineComponent), "line");
    PointComponent* pointComponents = allocateComponentMemory(sizeof(PointComponent), "point");

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
        entities[i].lightComponent = &lightComponents[i];
        initializeLightComponent(entities[i].lightComponent);
        entities[i].lineComponent = &lineComponents[i];
        initializeLineComponent(entities[i].lineComponent);
        entities[i].pointComponent = &pointComponents[i];
        initializePointComponent(entities[i].pointComponent);
    }

    globals.entities = entities;

    if(1){
        printf("lightComponent %zu\n",sizeof(LightComponent));
        printf("transformComponent %zu\n",sizeof(TransformComponent));
        printf("meshComponent %zu\n",sizeof(MeshComponent));
        printf("materialComponent %zu\n",sizeof(MaterialComponent));
        printf("groupComponent %zu\n",sizeof(GroupComponent));
        printf("uiComponent %zu\n",sizeof(UIComponent));
        printf("lineComponent %zu\n",sizeof(LineComponent));
        printf("pointComponent %zu\n",sizeof(PointComponent));
    }
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
void deleteEntity(Entity* entity){
    entity->alive = 0;
    entity->tag = UNINITIALIZED;
    entity->transformComponent->active = 0;
    entity->groupComponent->active = 0;
    entity->meshComponent->active = 0;
    entity->materialComponent->active = 0;
    entity->uiComponent->active = 0;
    entity->lightComponent->active = 0;
    entity->lineComponent->active = 0;
    entity->pointComponent->active = 0;
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
            globals.entities[i].uiComponent->boundingBox.x = globals.entities[i].transformComponent->position[0];
            globals.entities[i].uiComponent->boundingBox.y = globals.entities[i].transformComponent->position[1];
            globals.entities[i].uiComponent->boundingBox.width = globals.entities[i].transformComponent->scale[0];
            globals.entities[i].uiComponent->boundingBox.height = globals.entities[i].transformComponent->scale[1];

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
           
            if(globals.focusedEntityId != -1){
                //printf("input field is focused, ignoring keydown input from main loop\n");
                //printf("instead adding key to input field\n");
                return;
            }
           
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
            //printf("New Window size: %d x %d\n", w, h);
            recalculateViewports(w,h);
            
            // TODO: use this for reprojection later: float aspect = (float)w / (float)h;
            //glViewport(0, 0, w / 2, h);
        }
        if (globals.event.type == SDL_MOUSEBUTTONDOWN) {
            //printf("Mouse button pressed\n");
            // A button was pressed
            if (globals.event.button.button == SDL_BUTTON_LEFT) {
                // Left Button Pressed
                globals.mouseLeftButtonPressed = true;
                //printf("Left button pressed\n");
            } else if (globals.event.button.button == SDL_BUTTON_RIGHT) {
                // Left Button Released
                //printf("Right button pressed\n");
            }
        }  
        if (globals.event.type == SDL_MOUSEBUTTONUP) {
            // A button was released
            globals.mouseLeftButtonPressed = false;
            //printf("Mouse button released\n");
        }
        if (globals.event.type == SDL_MOUSEMOTION) {
            int xpos =  globals.event.motion.x;
            int ypos = globals.event.motion.y;

            globals.mouseXpos = (float)xpos;
            globals.mouseYpos = (float)ypos;

            // -----------TEMP CODE------------
           // printf("Mouse moved to %d, %d\n", xpos, ypos);
            // mouse move in ndc coordinates
            /*   float x_ndc = (2.0f * xpos) / width - 1.0f;
              float y_ndc = 1.0f - (2.0f * ypos) / height;
              float x_ui =  (-1 * x_ndc) * ((float)width * 0.5);
              float y_ui =   y_ndc * ((float)height * 0.5); */
              //printf("Mouse moved to NDC %f, %f\n", x_ndc, y_ndc);
              //printf("Mouse moved to %f, %f\n", mouseX, mouseY);
            //  Vector2 uiCoords = convertSDLToUI((float)xpos, (float)ypos);
            //    printf("Mouse moved in ui coords: %f, %f\n", uiCoords.x, uiCoords.y);
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


/**
 * @brief UI system
 * Responsible for setting the position of the UI elements in "UI" space. 
 * UI space is top left corner of the screen is [0.0,0.0], bottom right is [width,height].
 * Newly created UI elements is spawned in center of the screen [width/2, height/2] uiSystem then is responsible for 
 * setting the position to top left corner + the requested position.
 */
void uiSystem(){
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(
            globals.entities[i].alive == 1 
        && globals.entities[i].uiComponent->active 
        && globals.entities[i].transformComponent->active 
        && globals.entities[i].uiComponent->uiNeedsUpdate 
        && globals.cursorEntityId != i
        ) {
                
                // Goal here: Position element in "UI" space, where start 0,0 is the center of the ui-viewport screen. 
                // We will position the element to top left corner of the screen and treat this as 0,0 instead.
                // If position or scale changes, this calculation won't be correct anymore.
                // This calculation only works for elements that are spawned in the center of the screen.
                
                // Position of element on spawn:
                float ui_viewport_half_width = (float)globals.views.ui.rect.width / 2; 
                float ui_viewport_half_height = (float)globals.views.ui.rect.height / 2;
              //  printf("ui_viewport_half_width %f\n", ui_viewport_half_width);
              //  printf("ui_viewport_half_height %f\n", ui_viewport_half_height);
                
                // Half scale of element
                float scaleInPixelsX = globals.entities[i].transformComponent->scale[0]; 
                float scaleInPixelsY = globals.entities[i].transformComponent->scale[1]; /// globals.unitScale;
              //  printf("scaleInPixelsX %f\n", scaleInPixelsX);
              //  printf("scaleInPixelsY %f\n", scaleInPixelsY);

                // TODO: rotation
            
                // position of element
                float requested_pos_x = globals.entities[i].transformComponent->position[0];
                float requested_pos_y = globals.entities[i].transformComponent->position[1];

              //  printf("requested_pos_x %f\n", requested_pos_x);
              //  printf("requested_pos_y %f\n", requested_pos_y);

              //  printf("before %f %f\n", globals.entities[i].transformComponent->position[0], globals.entities[i].transformComponent->position[1]);
        
                // move element to upper left corner and then add requested position.
               globals.entities[i].transformComponent->position[0] = (float)(ui_viewport_half_width - (scaleInPixelsX * 0.5) - requested_pos_x) * -1.0; 
               globals.entities[i].transformComponent->position[1] =(float)(ui_viewport_half_height - (scaleInPixelsY * 0.5)) - requested_pos_y * 1.0;
             //  printf("final pos: %f %f\n", globals.entities[i].transformComponent->position[0], globals.entities[i].transformComponent->position[1]);
                
                // Bounding box
                globals.entities[i].uiComponent->boundingBox.x = requested_pos_x;
                globals.entities[i].uiComponent->boundingBox.y = requested_pos_y;
                globals.entities[i].uiComponent->boundingBox.width = globals.entities[i].transformComponent->scale[0];
                globals.entities[i].uiComponent->boundingBox.height = globals.entities[i].transformComponent->scale[1];
                
               /*  printf("bounding box x %d\n", globals.entities[i].uiComponent->boundingBox.x);
                printf("bounding box y %d\n", globals.entities[i].uiComponent->boundingBox.y);
                printf("bounding box width %d\n", globals.entities[i].uiComponent->boundingBox.width);
                printf("bounding box height %d\n", globals.entities[i].uiComponent->boundingBox.height);  
                printf("entity id %d\n", globals.entities[i].id);
                printf("bb entity to update %d\n", globals.entities[i].uiComponent->boundingBoxEntityId); */
                

                // Find the bounding box entity and update with new values
                for(int j = 0; j < MAX_ENTITIES; j++) {
                    if(globals.entities[j].alive == 1 && globals.entities[i].uiComponent->boundingBoxEntityId == globals.entities[j].id) { 
                        globals.entities[j].transformComponent->position[0] = globals.entities[i].transformComponent->position[0];
                        globals.entities[j].transformComponent->position[1] = globals.entities[i].transformComponent->position[1];
                        globals.entities[j].transformComponent->scale[0] = globals.entities[i].transformComponent->scale[0];
                        globals.entities[j].transformComponent->scale[1] = globals.entities[i].transformComponent->scale[1];
                        globals.entities[j].transformComponent->modelNeedsUpdate = 1;
                    }
                }
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
   // float radians = degrees * M_PI / 180.0f;

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
                if(globals.entities[i].lightComponent->active == 1){
                   // globals.entities[i].transformComponent->rotation[1] = radians;
                    globals.entities[i].transformComponent->modelNeedsUpdate = 1;
                   // float offset = 2.0 * sin(1.0 * globals.delta_time);
                  //  globals.entities[i].lightComponent->direction[0] = offset;
                    //printf("offset %f\n", offset);
                   // globals.entities[i].transformComponent->position[0] = offset;
                }
                //globals.entities[i].transformComponent->rotation[1] += radians; //<- This is an example of acceleration.
           }
        }
    }
}

void hoverAndClickSystem(){
    int newCursor = SDL_SYSTEM_CURSOR_ARROW;
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].transformComponent->active == 1 && globals.entities[i].uiComponent->active == 1){
                   
                    if(
                        globals.views.ui.isMousePointerWithin && 
                        isPointInsideRect(globals.entities[i].uiComponent->boundingBox, (vec2){ globals.mouseXpos, globals.mouseYpos})
                    ){
                       
                        // Left Click or just hover?
                        if(globals.mouseLeftButtonPressed){
                            if(strlen(globals.entities[i].uiComponent->text) > 0){
                               // printf("clicked\n");
                            }
                          
                            globals.entities[i].uiComponent->clicked = 1;
                        } else {
                            
                             if(strlen(globals.entities[i].uiComponent->text) > 0){
                              //  printf("hovered entity with id: %d\n", globals.entities[i].id);
                            }
                            globals.entities[i].uiComponent->hovered = 1;
                            globals.entities[i].uiComponent->clicked = 0;
                        }

                        // Hover effect
                        if(globals.entities[i].uiComponent->hovered == 1){
                            if(globals.entities[i].uiComponent->type == UITYPE_INPUT){
                                newCursor = SDL_SYSTEM_CURSOR_IBEAM;
                            }
                            if(globals.entities[i].uiComponent->type == UITYPE_BUTTON){
                                newCursor = SDL_SYSTEM_CURSOR_HAND;
                            }
                            if(strlen(globals.entities[i].uiComponent->text) > 0){
                              // printf("hovered changes done on entity with id %d\n", globals.entities[i].id);
                                // Appearance changes when hovered
                               globals.entities[i].materialComponent->diffuseMapOpacity = globals.entities[i].materialComponent->diffuseMapOpacity * 0.5f;
                               globals.entities[i].materialComponent->diffuse.r = 0.0f;
                               globals.entities[i].materialComponent->diffuse.g = 0.5f;
                            }
                        }

                        if(globals.entities[i].uiComponent->clicked == 1){
                           // printf("clicked entity with id: %d\n", globals.entities[i].id);
                            if(globals.entities[i].uiComponent->type == UITYPE_INPUT){
                                globals.focusedEntityId = globals.entities[i].id;
                               // printf("input field on entity with id %d clicked\n", globals.entities[i].id);
                            }else {
                                // If clicked ui element is not an input field
                                //if(globals.focusedEntityId != globals.entities[i].id)
                                    // Reset focused entity if there was one, same as onBlur.
                                   // printf("onblur/defocusing entity id %d\n", globals.focusedEntityId);
                                  //  globals.focusedEntityId = -1;
                                
                            }
                            if(strlen(globals.entities[i].uiComponent->text) > 0){
                                //printf("clicked changes done\n");
                            }
                            // Appearance changes when clicked
                            globals.entities[i].materialComponent->diffuseMapOpacity = globals.entities[i].materialComponent->diffuseMapOpacity * 0.5f;
                            globals.entities[i].materialComponent->diffuse.r = 0.5f;
                            globals.entities[i].materialComponent->diffuse.g = 0.0f;
                         
                            // Actions when clicked
                            if(globals.entities[i].uiComponent->onClick != NULL){
                                globals.entities[i].uiComponent->onClick();
                            }
                        }
                    
                    } else {
                         if(strlen(globals.entities[i].uiComponent->text) > 0){
                               // printf("no action,disable actions\n");
                            } 
                        globals.entities[i].uiComponent->hovered = 0;
                        globals.entities[i].uiComponent->clicked = 0;
                        globals.entities[i].materialComponent->diffuseMapOpacity = getMaterial(globals.entities[i].materialComponent->materialIndex)->diffuseMapOpacity;
                        globals.entities[i].materialComponent->diffuse.r = 0.0f;
                        globals.entities[i].materialComponent->diffuse.g= 0.0f;
                    }
            }
        }
    } 
    changeCursor(newCursor);
}

// TODO: temp solution, do not handle if caps lock is already active on program run.
bool isCapsLock() {
    SDL_Keymod modState = SDL_GetModState();
    if (modState & KMOD_CAPS) {
        //printf("Caps Lock is activated\n");
        return true;
    } else {
        //printf("Caps Lock is not activated\n");
        return false;
    }
}

// TODO: This is a temp solution. We should handle this using
// event->key.keysym.sym == SDLK_LSHIFT instead.
bool isLeftShiftPressed(){
    SDL_Keymod modState = SDL_GetModState();
    if (modState & KMOD_LSHIFT) {
        //printf("Left Shift is activated\n");
        return true;
    } else {
        //printf("Left Shift is not activated\n");
        return false;
    }
}

char toUpperCase(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 32;
    }
    return c; // Return the character unchanged if it's not a lowercase letter
}

char toLowerCase(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c; // Return the character unchanged if it's not an uppercase letter
}

char specialLeftShiftHandling(char c){
    if(c == '1'){
        return '!';
    }
    if(c == '2'){
        return '"';
    }
    if(c == '3'){
        return '#';
    }
    if(c == '4'){
        return '$';
    }
    if(c == '5'){
        return '%';
    }
    if(c == '6'){
        return '^';
    }
    if(c == '7'){
        return '&';
    }
    if(c == '8'){
        return '*';
    }
    if(c == '9'){
        return '(';
    }
    if(c == '0'){
        return ')';
    }
    return c;
}

/**
 * @brief UI input system
 * Handles input on UI elements.
 * TODO: memory leak, we are not deallocating the textCopy memory.
 * TODO: Support !?#
 * TODO: Support arrow keys for moving cursor
 * TODO: Support selecting text
 * TODO: Support remove selected text part
 * TODO: Support copy/paste
 * TODO: Support undo/redo
 * TODO: Support input validation
 * TODO: Support input mask
 * TODO: Support input type (number, email, password etc)
 * TODO: Support input placeholder
 * TODO: Support input focus indicator
 * TODO: Support values change from outside the input field
 * TODO: Support input field disabled state
 * BUG:  Empty field bugs out on input
 * BUG:  Too far typing to into the right of input field bugs out
 * BUG:  BackSpace in middle of a text should remove left character
 */
void uiInputSystem(){
    if(globals.focusedEntityId != -1){
         if(globals.event.type == SDL_KEYDOWN) {
            char* key = SDL_GetKeyName(globals.event.key.keysym.sym);
            printf("key %s\n", key);
            bool isSpaceKey = false;
            
            // Special keys
            if(strcmp(key, "Left") == 0){
                moveCursor(findCharacterUnderCursor().charWidth*-1.0);
                return;
            }
            if(strcmp(key, "Right") == 0){
                moveCursor(findCharacterUnderCursor().charWidth);
                return;
            }
            if(strcmp(key, "Space") == 0){
                isSpaceKey = true;
            }
            if(strcmp(key, "CapsLock") == 0){
                return;
            }
            if(strcmp(key, "Left Shift") == 0){
                return;
            }
            if(strcmp(key, "Delete") == 0){
                ASSERT(globals.focusedEntityId != -1, "No focused entity");
                handleDeleteButton(globals.focusedEntityId);
                return;
            }
            if(strcmp(key, "Return") == 0 || strcmp(key, "Escape") == 0){
                globals.focusedEntityId = -1;
                return;
            } 
            if(strcmp(key, "Backspace") == 0){
                if(strlen(globals.entities[globals.focusedEntityId].uiComponent->text) > 0){

                    // Remove the letter to the left of the cursor
                    removeCharacter(findCharacterUnderCursor().characterIndex-1);

                    // Move cursor one step to the left
                    Vector2 sdlVec = convertUIToSDL(globals.entities[globals.cursorEntityId].transformComponent->position[0], globals.entities[globals.cursorEntityId].transformComponent->position[1]);
                    ClosestLetter closestLetter = getClosestLetterInText(
                            globals.entities[globals.focusedEntityId].uiComponent,
                            sdlVec.x
                    );
                    Vector2 uiVec = convertSDLToUI(closestLetter.position.x, closestLetter.position.y);
                    globals.entities[globals.cursorEntityId].transformComponent->position[0] = uiVec.x;
                    globals.entities[globals.cursorEntityId].transformComponent->modelNeedsUpdate = 1;
                }
                return;
            }
                  
            // Any other key pressed
            ASSERT(strlen(globals.entities[globals.focusedEntityId].uiComponent->text) < 99, "Input field is full");
            
            // TODO: This is not deallocated , memory leak
            char* textCopy = (char*)arena_Alloc(&globals.uiArena, 99 * sizeof(char));
            
            // Add pressed key to input field & apply logic uppercase/lowercase/space
            bool doUpperCase = isLeftShiftPressed() ? isCapsLock() ? 0 : 1 : isCapsLock() ? 1 : 0;
            char keyCopy = doUpperCase == 1 ? toUpperCase(key[0]) : toLowerCase(key[0]);
            isSpaceKey ? keyCopy = 32 : keyCopy;
            isLeftShiftPressed() ? keyCopy = specialLeftShiftHandling(keyCopy) : keyCopy;

            // Find closest letter to cursor
            ClosestLetter closestLetter = findCharacterUnderCursor();
        
            // Use closest letter to insert key at the right position
            int j = 0;
            for(int i = 0; i < strlen(globals.entities[globals.focusedEntityId].uiComponent->text)+1; i++){

                if(i == (closestLetter.characterIndex)){
                    textCopy[i] = keyCopy;
                    j++;
                }

                textCopy[i+j] = globals.entities[globals.focusedEntityId].uiComponent->text[i];   
            }
            textCopy[strlen(globals.entities[globals.focusedEntityId].uiComponent->text)+2] = '\0';
            globals.entities[globals.focusedEntityId].uiComponent->text = textCopy;

            // Move cursor one step to the right
            Character ch = globals.characters[keyCopy];
            float advanceCursor = (float)(ch.Advance >> 6) * globals.charScale;
            globals.entities[globals.cursorEntityId].transformComponent->position[0] += advanceCursor;
            globals.entities[globals.cursorEntityId].transformComponent->modelNeedsUpdate = 1;

                
                
            
         }else {
           // printf("NO input \n");
         }
    }
}

Vector2 convertUIToSDL(float x, float y){
   // printf("ui x to convert: %f\n", x);
  //  printf("ui y to convert: %f\n", y);
    float sdl_x = (width / 2) - absValue(x);
    float sdl_y = (height / 2) - y;
    return (Vector2){sdl_x,sdl_y};
}

Vector2 convertSDLToUI(float x, float y){
    float x_ndc = (2.0f * x) / width - 1.0f;
    float y_ndc = 1.0f - (2.0f * y) / height;
    float x_ui = x_ndc * ((float)width * 0.5);
    float y_ui = y_ndc * ((float)height * 0.5);
    return (Vector2){x_ui, y_ui};
}

void moveCursor(float x){
    globals.entities[globals.cursorEntityId].transformComponent->position[0] += x;
    globals.entities[globals.cursorEntityId].transformComponent->modelNeedsUpdate = 1;
}
/**
 * @brief Removes the character on the index. Example: "TextIn|put" -> removeCharacter(6) "TextInut"
 */
void removeCharacter(int index){
    if(index < 0){
        return;
    }
    
    char* originalText = globals.entities[globals.focusedEntityId].uiComponent->text;
    int originalLength = strlen(originalText);
    
    // Allocate memory for the new string (original length - 1 character + null terminator)
    char* textCopy = (char*)malloc(originalLength * sizeof(char));
      if (textCopy == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    int j = 0;
    for(int i = 0; i < originalLength; i++){
        if(i == index){
            continue;
        }else{
            textCopy[j] = originalText[i];
            j++;
        }
    }
    textCopy[j] = '\0'; // null terminate
    
    globals.entities[globals.focusedEntityId].uiComponent->text = textCopy; // assign
}

/**
 * @brief Get the closest letter position in the text to the given mouse X position.
 * 
 * @param uiComponent The UI component containing the text.
 * @param mouseX The X position of the mouse.
 * @return Vector2 The position of the closest letter.
 */
ClosestLetter getClosestLetterInText(UIComponent* uiComponent, float mouseX){
    const char* text = uiComponent->text;
    float x = (float)uiComponent->boundingBox.x; 
    float y = (float)uiComponent->boundingBox.y + ((float)uiComponent->boundingBox.height / 2);   //(float)globals.characters[0].Size[1]; 
    float scale = globals.charScale; // Character size, also set when renderText is called (they should be in sync)
    float xpos = 0.0f;
    float ypos = 0.0f;
    float bestCharX = (float)uiComponent->boundingBox.x + (float)uiComponent->boundingBox.width;
    float lastShift = 0.0f;
    ClosestLetter closestLetter;
    closestLetter.characterIndex = 0;
    closestLetter.position = (Vector2){0.0f, 0.0f};
    if(strlen(text) == 0){
        return closestLetter;
    }
        
    for (unsigned char c = 0; c < strlen(text); c++) {
        Character ch = globals.characters[text[c]];

        // Calculate the position of the current character
        xpos = x + (float)ch.Bearing[0] * scale;
        ypos = y - ((float)ch.Size[1] - (float)ch.Bearing[1]) * scale;

        float w = (float)ch.Size[0] * scale;
        float h = (float)ch.Size[1] * scale;    
       
        // Check if the mouse is closer to the current character than the previous closest character
        float testxpos = absValue(mouseX - xpos);
        float testbestCharX = absValue(mouseX - bestCharX);
           
        if(testxpos > bestCharX){
            // Previous character was the closest,so we remove the last character width from the xpos.
            closestLetter.position.x = xpos - (float)ch.Bearing[0] * scale - lastShift;
            closestLetter.position.y = ypos;
            closestLetter.characterIndex = c - 1; 
            closestLetter.charWidth = (float)ch.Bearing[0] * scale + lastShift;
            ASSERT(closestLetter.position.x >= 0, "closestLetter.position.x is negative");
            ASSERT(closestLetter.position.y >= 0, "closestLetter.position.y is negative");
            ASSERT(closestLetter.characterIndex >= 0, "closestLetter.characterIndex is negative");
            ASSERT(closestLetter.charWidth >= 0, "closestLetter.charWidth is negative");
            return closestLetter;
        }
        bestCharX = testxpos;
   
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        lastShift = (float)(ch.Advance >> 6) * scale;
        x += lastShift; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        closestLetter.characterIndex++;
        closestLetter.charWidth = (float)ch.Bearing[0] * scale + lastShift; 
   } 
   
   // Determine which side of the character is closest to the mouse between last character and penultimate character.
   float lastCharPos = absValue(mouseX - (xpos + lastShift));
   float prevCharPos = absValue(mouseX - xpos);
   float result = prevCharPos > lastCharPos ? (xpos+lastShift) : xpos; 

   closestLetter.position.x = result;
   closestLetter.position.y = ypos;
   closestLetter.characterIndex = prevCharPos > lastCharPos ? closestLetter.characterIndex : (closestLetter.characterIndex - 1);
            
   ASSERT(closestLetter.position.x >= 0, "closestLetter.position.x is negative(e)");
   ASSERT(closestLetter.position.y >= 0, "closestLetter.position.y is negative(e)");
   ASSERT(closestLetter.characterIndex >= 0, "closestLetter.characterIndex is negative(e)");
   ASSERT(closestLetter.charWidth >= 0, "closestLetter.charWidth is negative(e)");

   return closestLetter;
}

ClosestLetter findCharacterUnderCursor(){
    // Find out where the cursor is in the text
    // Find the closest letter to the cursor (getClosestLetterInText) probably need to also return the index of the letter in the text and its char-width.
    ASSERT(globals.focusedEntityId != -1, "No focused entity");
    ASSERT(globals.cursorEntityId != -1, "No cursor entity");
 
    Vector2 sdlVec = convertUIToSDL(globals.entities[globals.cursorEntityId].transformComponent->position[0], globals.entities[globals.cursorEntityId].transformComponent->position[1]);
  
    ASSERT(sdlVec.x >= 0, "Sdl cursor x position is negative");
    ASSERT(sdlVec.y >= 0, "Sdl cursor y position is negative");
    return getClosestLetterInText(
                    globals.entities[globals.focusedEntityId].uiComponent,
                    sdlVec.x
        );
}

/**
 * @brief Delete the character to the left of the cursor.
 * Triggers on backspace key press.
 */
void handleDeleteButton(){
    if(strlen(globals.entities[globals.focusedEntityId].uiComponent->text) == 0){
        return;
    }
    ClosestLetter closestLetter = findCharacterUnderCursor();
                 
    // Remove the letter to the left of the cursor
    removeCharacter(closestLetter.characterIndex);
}


// Select all text fn
void selectAllText(){

}

void textCursorSystem(){
    if(globals.focusedEntityId != -1){
  
        // Create cursor
        if(globals.cursorEntityId == -1){
            ClosestLetter closestLetter = getClosestLetterInText(globals.entities[globals.focusedEntityId].uiComponent, globals.mouseXpos);         
            Vector2 uiVec = convertSDLToUI(closestLetter.position.x, closestLetter.position.y);
            globals.cursorEntityId = ui_createRectangle(globals.materials[0], (vec3){uiVec.x, uiVec.y, 2.0f}, (vec3){3.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
        }
        
        // Blink cursor logic
        if((int)globals.delta_time % 2 == 0){
            globals.entities[globals.cursorEntityId].uiComponent->active = 0;
        }else {
            globals.entities[globals.cursorEntityId].uiComponent->active = 1;
        }
    }else{
        if(globals.cursorEntityId != -1){
            deleteEntity(&globals.entities[globals.cursorEntityId]);
            globals.cursorEntityId = -1;
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
                
                    // rotate model on x, y, and z axes
                    // The cast on model tells the compiler that you're aware of the 
                    // const requirement and that you're promising not to modify the model matrix.
                    mat4x4 rotatedModelX, rotatedModelY, rotatedModelZ;
                    mat4x4_rotate_X(rotatedModelX, (const float (*)[4])model, globals.entities[i].transformComponent->rotation[0]);
                    mat4x4_rotate_Y(rotatedModelY, (const float (*)[4])rotatedModelX, globals.entities[i].transformComponent->rotation[1]);
                    mat4x4_rotate_Z(rotatedModelZ, (const float (*)[4])rotatedModelY, globals.entities[i].transformComponent->rotation[2]);

                    // Copy the rotated model matrix to the transform component
                    memcpy(globals.entities[i].transformComponent->transform, rotatedModelZ, sizeof(mat4x4));

                    globals.entities[i].transformComponent->modelNeedsUpdate = 0;
           }
          }}
}

void debugSystem(){
    
    // Turn off debug draw calls, we only want one frame of drawcalls saved.
    if(globals.debugDrawCalls){
        globals.debugDrawCalls = false;
    }
       
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
    uiInputSystem();
    hoverAndClickSystem();
    textCursorSystem();
   // movementSystem();
    modelSystem();
   
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

    // Native

    // TODO: find a way to avoid having to iterate over all entities twice, once for ui and once for 3d objects.

    // Render main view & 3d objects
   setViewportAndClear(globals.views.full);
   setFontProjection(&globals.gpuFontData,globals.views.full);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1) {
                if(globals.entities[i].uiComponent->active != 1){
                    renderMesh(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,globals.views.main.camera,globals.entities[i].materialComponent);
                }
            }
        }
    }
    // Render GL_LINES & GL_POINTS(particles)
    for(int i = 0; i < MAX_ENTITIES; i++){
        if((globals.entities[i].alive == 1 && globals.entities[i].lineComponent->active == 1) || globals.entities[i].pointComponent->active == 1){
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
        if(globals.entities[i].alive == 1) {
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
        if(globals.entities[i].alive == 1) {
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
                            globals.charScale,(Color){1.0f, 1.0f, 0.0f});
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

/**
 * @brief Initialize the font
 * Load texture,setup mesh,setup material
 * After this the projection matrix for the font need to be set using setFontProjection.
 * And then you can render text using renderText.
 */
void initFont(){
    setupFontTextures("./Assets/ARIAL.TTF",48);
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
 //  TextureData oldBricksTest = loadTexture("./Assets/oldbricks.jpg");
   TextureData containerTextureData = loadTexture("./Assets/container.jpg");
   GLuint containerMap = setupTexture(containerTextureData);
   TextureData containerTwoTextureData = loadTexture("./Assets/container2.png");
   GLuint containerTwoMap = setupTexture(containerTwoTextureData);
   TextureData containerTwoSpecTextureData = loadTexture("./Assets/container2_specular.png");
   GLuint containerTwoSpecularMap = setupTexture(containerTwoSpecTextureData);

   
   
    //ObjGroup* truck = obj_loadFile("./Assets/truck.obj"); // Not supported atm, need .obj group support.
    ObjGroup* cornell_box = obj_loadFile("./Assets/cornell_box.obj");  
  /*    ObjGroup* bunny = obj_loadFile("./Assets/bunny2.obj");
    ObjGroup* plane = obj_loadFile("./Assets/plane.obj"); 
    ObjGroup* objExample = obj_loadFile("./Assets/Two_adjoining_squares_with_vertex_normals.obj");
    ObjGroup* sphere = obj_loadFile("./Assets/blender_sphere3.obj");
    ObjGroup* triangleVolumes = obj_loadFile("./Assets/triangle_volumes.obj");
    ObjGroup* teapot = obj_loadFile("./Assets/teapot.obj");
    ObjGroup* dragon = obj_loadFile("./Assets/dragon.obj");   */
    ObjGroup* textured_objects = obj_loadFile("./Assets/textured_objects.obj");
 
    struct Material objectMaterial = {
    .active = 1,
    .name = "objectMaterial",
    .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
    .diffuse = (Color){1.0f, 0.0f, 0.0f, 1.0f},  // used when diffuseMapOpacity lower than 1.0
    .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
    .shininess = 32.0f,                           // used
    .diffuseMap = containerTwoMap,               // used
    .diffuseMapOpacity = 1.0f,                  // used
    .specularMap = containerTwoSpecularMap,      // used
    };
    struct Material uiMaterial = {
        .active = 1,
        .name = "uiMaterial",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = (Color){0.5f, 0.5f, 0.0f, 1.0f},  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 1.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    };
    struct Material flatColorUiDarkGrayMat = {
        .active = 1,
        .name = "flatColorUiDarkGrayMat",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = DARK_GRAY_COLOR,  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 0.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    };
    struct Material flatColorUiGrayMat = {
        .active = 1,
        .name = "flatColorUiGrayMat",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = GRAY_COLOR,  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 0.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    };
    struct Material lightMaterial = {
        .active = 1,
        .name = "lightMaterial",
        .ambient = (Color){1.0f, 1.0f, 1.0f, 1.0f},  // used
        .diffuse = (Color){1.0f, 1.0f, 1.0f, 1.0f},  // used
        .specular = (Color){1.0f, 1.0f, 1.0f, 1.0f}, // used
        .shininess = 32.0f,                         // NOT used
        .diffuseMap = containerMap,                  // NOT used
        .diffuseMapOpacity = 1.0f,                  // NOT used
        .specularMap = containerMap,                 // NOT used
    };

   
    // Main viewport objects (3d scene) x,y,z coords is a world space coordinate (not yet implemented?).
    createObject(&cornell_box->objData[0],(vec3){2.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[1],(vec3){2.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[2],(vec3){2.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[3],(vec3){2.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[4],(vec3){2.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[5],(vec3){2.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[6],(vec3){2.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[7],(vec3){2.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});  
 
    createObject(&textured_objects->objData[0],(vec3){0.0f, -2.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});  
    createObject(&textured_objects->objData[1],(vec3){0.0f, -2.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
   
 // createObject(VIEWPORT_MAIN,&plane->objData[0],(vec3){5.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
   // createObject(VIEWPORT_MAIN,&bunny->objData[0],(vec3){6.0f, 0.0f, 0.0f}, (vec3){10.0f, 10.0f, 10.0f}, (vec3){0.0f, 0.0f, 0.0f});   
   // createObject(VIEWPORT_MAIN,&truck->objData[0],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
   //  createObject(VIEWPORT_MAIN,&objExample->objData[0],(vec3){5.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&sphere->objData[0],(vec3){3.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&triangleVolumes->objData[0],(vec3){4.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&teapot->objData[0],(vec3){0.0f, 0.0f, 0.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){-90.0f, 0.0f, 0.0f}); 
    //createObject(VIEWPORT_MAIN,&dragon->objData[0],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});     

    // lights
    createLight(lightMaterial,(vec3){-1.0f, 1.0f, 1.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},SPOT);
    createLight(lightMaterial,(vec3){-1.0f, 1.0f, 1.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},SPOT);
    createLight(lightMaterial,(vec3){-1.0f, 1.0f, 1.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},SPOT);
    createLight(lightMaterial,(vec3){-1.0f, 1.0f, 1.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},SPOT);
    createLight(lightMaterial,(vec3){-1.0f, 1.0f, 1.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},DIRECTIONAL);
    createLight(lightMaterial,(vec3){0.7f, 0.2f, 2.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);
    createLight(lightMaterial,(vec3){1.7f, 0.2f, 2.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);
    createLight(lightMaterial,(vec3){2.7f, 0.2f, 2.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);
    createLight(lightMaterial,(vec3){3.7f, 0.2f, 2.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);  

    // Primitives
    createPlane(objectMaterial, (vec3){0.0f, -1.0f, 0.0f}, (vec3){5.0f, 5.0f, 5.0f}, (vec3){90.0f, 0.0f, 0.0f});
    createCube(objectMaterial,(vec3){2.0f, -0.0f, -0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
  


   // UI scene objects creation (2d scene) x,y coords where x = 0 is left and y = 0 is top and x,y is pixel positions. (controlled by the uiSystem)
   // Scale is in pixels, 100.0f is 100 pixels etc.
   // z position will be z-depth, much like in DOM in web.Use this to control draw order.
   // TODO: implement rotation, it is atm not affecting. 
 
  // UI Settings Panel
  ui_createButton(flatColorUiGrayMat, (vec3){545.0f, 5.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Header",onButtonClick);
  ui_createTextInput(flatColorUiGrayMat, (vec3){200.0f, 55.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "TextInput",onTextInputChange);
    //ui_slider(flatColorUiDarkGrayMat, (vec3){545.0f, 55.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Slider",onSliderChange);
    //ui_colorPicker(flatColorUiDarkGrayMat, (vec3){545.0f, 105.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "ColorPicker",onColorPickerChange);
    ui_createRectangle(flatColorUiGrayMat, (vec3){545.0f, 55.0f, 0.0f}, (vec3){10.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    ui_createRectangle(flatColorUiDarkGrayMat, (vec3){555.0f, 55.0f, 1.0f}, (vec3){240.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
  ui_createButton(flatColorUiGrayMat, (vec3){545.0f, 355.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Header2",onButtonClick);
    ui_createRectangle(flatColorUiGrayMat, (vec3){545.0f, 405.0f, 0.0f}, (vec3){10.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    ui_createRectangle(flatColorUiDarkGrayMat, (vec3){555.0f, 405.0f, 1.0f}, (vec3){240.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});


 //ui_createRectangle(uiMaterial, (vec3){765.0f, 5.0f, 0.0f}, (vec3){35.0f, 50.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f});

 // Textured button
 //ui_createButton(uiMaterial, (vec3){150.0f, 0.0f, 0.0f}, (vec3){150.0f, 50.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Rotate",onButtonClick);
  
   
/*     printf("materials-list (%d): \n",globals.materialsCount);
    for(int i = 0; i < globals.materialsCount; i++){
       // printf("%s \n",globals.materials[i].name);
         printf("shininess %f \n",globals.materials[i].shininess);
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
        printf("shininessMap %d \n\n",globals.materials[i].shininessMap);  
   }  */

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
    GLenum drawMode,
    VertexDataType vertexDataType,
    Entity* entity,
    bool saveMaterial // save material to global list
    ){

    entity->meshComponent->active = 1;

    // vertex data
    entity->meshComponent->vertices = verts;
    
    int stride = 11;
    int vertexIndex = 0;

    // We have three types of vertex data input:
    // - one with color & indices VERT_COLOR_INDICIES
    // - one with color & no indicies VERT_COLOR
    // - one with no color & no indicies VERT
    if(numIndicies == 0 && vertexDataType == VERTS_ONEUV) {

        // vertices + texcoords but no indices and no color
        stride = 11;
        for(int i = 0; i < num_of_vertex * stride; i+=stride) {
            entity->meshComponent->vertices[vertexIndex].position[0] = verts[i];
            entity->meshComponent->vertices[vertexIndex].position[1] = verts[i + 1];
            entity->meshComponent->vertices[vertexIndex].position[2] = verts[i + 2];

            entity->meshComponent->vertices[vertexIndex].color[0] = verts[i + 3];
            entity->meshComponent->vertices[vertexIndex].color[1] = verts[i + 4];
            entity->meshComponent->vertices[vertexIndex].color[2] = verts[i + 5]; 

            entity->meshComponent->vertices[vertexIndex].texcoord[0] = verts[i + 6];
            entity->meshComponent->vertices[vertexIndex].texcoord[1] = verts[i + 7];

            entity->meshComponent->vertices[vertexIndex].normal[0] = verts[i + 8];
            entity->meshComponent->vertices[vertexIndex].normal[1] = verts[i + 9];
            entity->meshComponent->vertices[vertexIndex].normal[2] = verts[i + 10];

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
            entity->meshComponent->vertices[vertexIndex].normal[0] = verts[i + 8];
            entity->meshComponent->vertices[vertexIndex].normal[1] = verts[i + 9];
            entity->meshComponent->vertices[vertexIndex].normal[2] = verts[i + 10];
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

            entity->meshComponent->vertices[vertexIndex].normal[0] = verts[i + 8];
            entity->meshComponent->vertices[vertexIndex].normal[1] = verts[i + 9];
            entity->meshComponent->vertices[vertexIndex].normal[2] = verts[i + 10];

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

   /*  if(numIndicies == 0){
        printf("not using indicies\n");
    } */

    // transform data
    entity->transformComponent->active = 1;
    entity->transformComponent->position[0] = position[0];
    entity->transformComponent->position[1] = position[1];
    entity->transformComponent->position[2] = position[2];
    entity->transformComponent->scale[0] = scale[0];
    entity->transformComponent->scale[1] = scale[1];
    entity->transformComponent->scale[2] = scale[2];
    entity->transformComponent->rotation[0] = rotation[0] * M_PI / 180.0f; // convert to radians
    entity->transformComponent->rotation[1] = rotation[1] * M_PI / 180.0f;
    entity->transformComponent->rotation[2] = rotation[2] * M_PI / 180.0f;
    entity->transformComponent->modelNeedsUpdate = 1;

    // material data
    entity->materialComponent->active = 1;
    entity->materialComponent->ambient = material->ambient;
    entity->materialComponent->diffuse = material->diffuse;
    entity->materialComponent->specular = material->specular;
    entity->materialComponent->shininess = material->shininess;
    entity->materialComponent->diffuseMap = material->diffuseMap;
    entity->materialComponent->diffuseMapOpacity = material->diffuseMapOpacity;
    entity->materialComponent->specularMap = material->specularMap;
    if(saveMaterial){
        entity->materialComponent->materialIndex = addMaterial(*material);
    }

    entity->meshComponent->gpuData->drawMode = drawMode;
    
    setupMesh(  entity->meshComponent->vertices, 
                entity->meshComponent->vertexCount, 
                entity->meshComponent->indices, 
                entity->meshComponent->indexCount,
                entity->meshComponent->gpuData
                );
    
    if(entity->lightComponent->active == 1){
        setupMaterial( entity->meshComponent->gpuData,"shaders/light_vertex.glsl", "shaders/light_fragment.glsl" );
    }else if(entity->uiComponent->active == 1) {
        setupMaterial( entity->meshComponent->gpuData,"shaders/ui_vertex.glsl", "shaders/ui_fragment.glsl" );
    }else {
        setupMaterial( entity->meshComponent->gpuData,"shaders/mesh_vertex.glsl", "shaders/mesh_fragment.glsl" );
    }
    
}

void createPoint(vec3 position){
    GLfloat vertices[] = {
        position[0], position[1], position[2],  1.0f, 1.0f, 1.0f, 0.0f, 0.0f
    };
}

int addMaterial(Material material){
    globals.materials[globals.materialsCount] = material;
    globals.materialsCount++;
    return globals.materialsCount-1;
}
Material* getMaterial(int index){
    ASSERT(index < globals.materialsCount && index >= 0, "Material index out of bounds or not assigned");
    return &globals.materials[index];
}
     
/**
 * @brief Create a light
 * Create a light source in the scene.
 * 
 */
void createLight(Material material,vec3 position,vec3 scale,vec3 rotation,vec3 direction,LightType type){
    
   // vertex data
    GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  1.0f,  0.0f
};
    // index data NOT USED ATM and not correct anymore
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
    
    // cutOff
    float degrees = 12.5f;
    float radians = DEG_TO_RAD(degrees);
    float cosine = cosf(radians);

    // outerCutOff
    float degreesOC = 17.5f;
    float radiansOC = DEG_TO_RAD(degreesOC);
    float cosineOC = cosf(radiansOC);

    Entity* entity = addEntity(MODEL);
    entity->lightComponent->active = 1;
    entity->lightComponent->direction[0] = direction[0];
    entity->lightComponent->direction[1] = direction[1];
    entity->lightComponent->direction[2] = direction[2];
    entity->lightComponent->intensity = 1.0f;
    entity->lightComponent->diffuse = material.diffuse;
    entity->lightComponent->specular = material.specular;
    entity->lightComponent->ambient = material.ambient;
    entity->lightComponent->constant = 1.0f;
    entity->lightComponent->linear = 0.09f;
    entity->lightComponent->quadratic = 0.032f;
    entity->lightComponent->cutOff = cosine;
    entity->lightComponent->outerCutOff = cosineOC;
    entity->lightComponent->type = type;
    
    // TODO: This is a temporary solution, need to implement a better way to handle lights.
    globals.lights[globals.lightsCount] = *entity;
    ASSERT(globals.lightsCount < MAX_LIGHTS, "Too many lights");
    globals.lightsCount++;

    if(type == DIRECTIONAL){
        vec3 result;
        vec3 normalized_direction;
        vec3_norm(&normalized_direction,direction);
        vec3_add(&result,normalized_direction,position);
        createLine(position,result,entity);
        GLfloat positions[] = {  position[0], position[1], position[2] ,  result[0], result[1], result[2]  };
        createPoints(positions,2,entity);
        return;
    }
    createMesh(vertices,36,indices,0,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
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
 * @brief Create a line segment between two points
 * @param position - start position
 * @param endPosition - end position
 */
void createLine(vec3 position, vec3 endPosition,Entity* entity){
    entity->lineComponent->active = 1;
    entity->lineComponent->start[0] = position[0];
    entity->lineComponent->start[1] = position[1];
    entity->lineComponent->start[2] = position[2];
    entity->lineComponent->end[0] = endPosition[0];
    entity->lineComponent->end[1] = endPosition[1];
    entity->lineComponent->end[2] = endPosition[2];
    entity->lineComponent->color = (Color){1.0f,1.0f,0.0f,1.0f};
    // TODO: transform pos is start of segment, should it be mid-point?
    entity->transformComponent->position[0] = position[0];
    entity->transformComponent->position[1] = position[1];
    entity->transformComponent->position[2] = position[2];
    entity->transformComponent->scale[0] = 1.0f;
    entity->transformComponent->scale[1] = 1.0f;
    entity->transformComponent->scale[2] = 1.0f;
    entity->transformComponent->rotation[0] = 0.0f;
    entity->transformComponent->rotation[1] = 0.0f;
    entity->transformComponent->rotation[2] = 0.0f;
    entity->transformComponent->modelNeedsUpdate = 1;

    GLfloat lines[] = {
        position[0], position[1], position[2], 
        endPosition[0], endPosition[1], endPosition[2]
    };

    setupLine(lines,2,entity->lineComponent->gpuData);
    setupMaterial(entity->lineComponent->gpuData,"shaders/line_vertex.glsl", "shaders/line_fragment.glsl");
}

void createPlane(Material material,vec3 position,vec3 scale,vec3 rotation){
    // vertex data
    GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords    // Normals
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  0.0f, 0.0f, 1.0f,    // Bottom-left
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,  0.0f, 0.0f, 1.0f,    // Bottom-right
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,  0.0f, 0.0f, 1.0f,    // Top-right
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,  0.0f, 0.0f, 1.0f,    // Top-right
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,  0.0f, 0.0f, 1.0f,    // Top-left
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  0.0f, 0.0f, 1.0f,    // Bottom-left
};
    // index data NOT USED ATM
    GLuint indices[] = {
    0, 1, 2, // First triangle
    2, 3, 0  // Second triangle
    };
   
    Entity* entity = addEntity(MODEL);

    createMesh(vertices,6,indices,0,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
}
/**
 * @brief Create a object. Used together with obj-load/parse. Expects data from obj-parser to be of type ObjData.
 * Create a object mesh
 * @param diffuse - color of the rectangle
*/
void createObject(ObjData* obj,vec3 position,vec3 scale,vec3 rotation){
    
   // vertex data
    int stride = 11;
  
    // NOT USED
    GLuint indices[] = {
        0, 1, 2,  // first triangle
        1, 2, 3   // second triangle
    }; 

    Entity* entity = addEntity(MODEL);
    
    createMesh(obj->vertexData,obj->num_of_vertices,indices,0,position,scale,rotation,&globals.materials[obj->materialIndex],GL_TRIANGLES,VERTS_COLOR_ONEUV,entity,false);
}

/**
 * TODO: This will create a ui_panel and is in turn an abstraction to create a group of ui-elements to make up this panel.
 */
void ui_panel(Material material, vec3 position,vec3 scale, vec3 rotation,Rectangle rect, bool floating, bool collapsable){
  //ui_createRectangle(Material material, vec3 positi)
  printf("not yet implemented \n");
}

/**
 * @brief Create a rectangle
 * Create a rectangle mesh
 * @param diffuse - color of the rectangle
*/
int ui_createRectangle(Material material,vec3 position,vec3 scale,vec3 rotation){
    // vertex data
    GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords    // Normals
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
};
    //OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(MODEL);
    entity->uiComponent->active = 1;
    entity->uiComponent->boundingBox = (Rectangle){0.0f,0.0f,0.0f,0.0f};
    entity->uiComponent->uiNeedsUpdate = 1;
    
    createMesh(vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true);

    return entity->id;
}
/**
 * @brief Create a button
 * Create a button mesh in ui
 * @param diffuse - color of the rectangle
*/
void ui_createButton(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,ButtonCallback onClick){
    // vertex data
        GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords    // Normals
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
};
    // OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(MODEL);
    
    entity->uiComponent->active = 1;
    entity->uiComponent->boundingBox = (Rectangle){0.0f,0.0f,0.0f,0.0f};
    entity->uiComponent->text = text;
    entity->uiComponent->uiNeedsUpdate = 1;
    entity->uiComponent->onClick = onClick;
    entity->uiComponent->type = UITYPE_BUTTON;
    
    createMesh(vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 
}
/**
 * @brief Create a input field
 * Create a input mesh in ui
 * @param diffuse - color of the rectangle
*/
void ui_createTextInput(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,OnChangeCallback onChange){
    // vertex data
        GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords    // Normals
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
};
    // OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(MODEL);
    
    entity->uiComponent->active = 1;
    entity->uiComponent->boundingBox = (Rectangle){0.0f,0.0f,0.0f,0.0f};
    entity->uiComponent->text = text;
    entity->uiComponent->uiNeedsUpdate = 1;
    entity->uiComponent->onChange = onChange;
    entity->uiComponent->type = UITYPE_INPUT;
    
    createMesh(vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 
}

/**
 * @brief Create a Cube
 * Create a Cube mesh
 * @param diffuse - color of the cube
*/
void createCube(Material material,vec3 position,vec3 scale,vec3 rotation){
    // vertex data
    GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  1.0f,  0.0f
};
    // index data NOT USED ATM and not correct anymore
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

    Entity* entity = addEntity(MODEL);
    
    createMesh(vertices,36,indices,0,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
     
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
    camera->speed = 0.01f;
    camera->viewMatrixNeedsUpdate = 1;
    camera->projectionMatrixNeedsUpdate = 1;
    camera->fov = 45.0f;
    camera->near = 0.1f;
    camera->far = 100.0f;
    camera->aspectRatio = (float)width / (float)height;
    camera->left = -1.0f;
    camera->right = 1.0f;
    camera->bottom = -1.0f;
    camera->top = 1.0f;
    camera->isOrthographic = 0;

    return camera;
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
void onTextInputChange(){
    printf("Text input changed!\n");
}

// -----------------------------------------------------
// ASSETS
// -----------------------------------------------------



    
