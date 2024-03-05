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

#include "opengl.h"
#include "utils.h"
#include <time.h>
#include "types.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Utility macros
#define CHECK_ERROR(test, message) \
    do { \
        if((test)) { \
            fprintf(stderr, "%s\n", (message)); \
            exit(1); \
        } \
    } while(0)

// Get a random number from 0 to 255
int randInt(int rmin, int rmax) {
    return rand() % rmax + rmin;
}
float randFloat(float rmin, float rmax) {
    return (float)rand() / (float)RAND_MAX * (rmax - rmin) + rmin;
}

// Globals
struct Globals {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;
    SDL_GLContext gl_context;
    int running;
    Entity* entities;
    int vertex_count;
    float delta_time;
};
struct Globals globals = {.window=NULL,.renderer=NULL,.event={0},.gl_context=NULL,.running=1,.entities=NULL,.vertex_count=0,.delta_time=0.0f};


struct View views[3] = {
    {0, 200, 800, 400},
    {0, 0, 800, 200},
    {0, 0, 800, 600}
};

// Window dimensions
static const int width = 800;
static const int height = 600;

void setRenderDrawColor(int r,int g, int b, int a) {
    // Initial renderer color
    SDL_SetRenderDrawColor(globals.renderer, r, g, b, a);
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
    CHECK_ERROR(SDL_Init(SDL_INIT_VIDEO) != 0, SDL_GetError());

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
    CHECK_ERROR(globals.window == NULL, SDL_GetError());

}

void setViewport(struct View view) {
    glViewport(view.x, view.y, view.width, view.height);
}

//------------------------------------------------------
//  ECS
//------------------------------------------------------

// Max entities constant
#define MAX_ENTITIES 100000

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
    transformComponent->position.x = 0.0f;
    transformComponent->position.y = 0.0f;
    transformComponent->position.z = 0.0f;
    transformComponent->scale.x = 1.0f;
    transformComponent->scale.y = 1.0f;
    transformComponent->scale.z = 1.0f;
    transformComponent->rotation.x = 0.0f;
    transformComponent->rotation.y = 0.0f;
    transformComponent->rotation.z = 0.0f;
    transformComponent->isDirty = 1;
}

void initializeMatrix4(Matrix4* matrix) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            matrix->m[i][j] = 0.0f;
        }
    }
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
}

void initializeUIComponent(UIComponent* uiComponent){
    uiComponent->active = 0;
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
//  PROGRAM ACTIONS
//------------------------------------------------------

/**
 * @brief Create a mesh
 * Main function to create a mesh. 
 *  - vertex data
 *  - index data
 *  - transform data
 *  - material data
*/
void createMesh(float* verts,int num_of_vertex, unsigned int* indices,int numIndicies,Vector3* position,Vector3* scale,Vector3* rotation,Material* material,int ui){

    Entity* entity = addEntity(MODEL);
    
    entity->meshComponent->active = 1;
    if(ui == 1){
        entity->uiComponent->active = 1;
    }

    // vertex data
    entity->meshComponent->vertices = (Vertex*)malloc(num_of_vertex * sizeof(Vertex));
     if (entity->meshComponent->vertices == NULL) {
        printf("Failed to allocate memory for vertices\n");
        exit(1);
    }
    for(int i = 0; i < num_of_vertex; i++) {
        entity->meshComponent->vertices[i].position.x = verts[i * 3];
        entity->meshComponent->vertices[i].position.y = verts[i * 3 + 1];
        entity->meshComponent->vertices[i].position.z = verts[i * 3 + 2];
    }
    entity->meshComponent->vertexCount = num_of_vertex;

    // index data
    entity->meshComponent->indices = (unsigned int*)malloc(6 * sizeof(unsigned int));
    if(entity->meshComponent->indices == NULL) {
        printf("Failed to allocate memory for indices\n");
        exit(1);
    }
    for(int i = 0; i < 6; i++) {
        entity->meshComponent->indices[i] = indices[i];
    }
    entity->meshComponent->indexCount = numIndicies;

    // transform data
    entity->transformComponent->active = 1;
    entity->transformComponent->position.x = position->x;
    entity->transformComponent->position.y = position->y;
    entity->transformComponent->position.z = position->z;
    entity->transformComponent->scale.x = scale->x;
    entity->transformComponent->scale.y = scale->y;
    entity->transformComponent->scale.z = scale->z;
    entity->transformComponent->rotation.x = rotation->x;
    entity->transformComponent->rotation.y = rotation->y;
    entity->transformComponent->rotation.z = rotation->z;
    entity->transformComponent->isDirty = 1;

    // material data
    entity->materialComponent->active = 1;
    entity->materialComponent->ambient = material->ambient;
    entity->materialComponent->diffuse = material->diffuse;
    entity->materialComponent->specular = material->specular;
    entity->materialComponent->shininess = material->shininess;
   

    setupMesh(  entity->meshComponent->vertices, 
                entity->meshComponent->vertexCount, 
                entity->meshComponent->indices, 
                entity->meshComponent->indexCount,
                entity->meshComponent->gpuData );

    setupMaterial( entity->meshComponent->gpuData ); 
}

void createTriangle(int ui,Color diffuse){
    // vertex data
    float verts[] = {
         0.0f,  0.5f, 0.0f,  // top
        -0.5f, -0.5f, 0.0f,  // bottom left
         0.5f, -0.5f, 0.0f   // bottom right
    };
    // index data
    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        1, 2, 3   // second triangle
    }; 
    // transform
    Vector3 position = {0.0f, 0.0f, 0.0f};
    Vector3 scale = {1.0f, 1.0f, 1.0f};
    Vector3 rotation = {0.0f, 0.0f, 0.0f};

    //material
    Color ambient = {0.1f, 0.1f, 0.1f, 1.0f};
    //Color diffuse = diffuseColor;
    Color specular = {0.6f, 0.6f, 0.6f, 1.0f};
    float shininess = 32.0f;
    Material material = {ambient, diffuse, specular, shininess};

    createMesh(verts,3,indices,6,&position,&scale,&rotation,&material,ui);
}

void createRectangle(int ui,Color diffuse){
    // vertex data
    GLfloat vertices[] = {
    // triangle one
    -1.0f, -1.0f, 0.0f,  // bottom left

    // shared vertices
    -1.0f,  1.0f, 0.0f,  // top left
     1.0f, -1.0f, 0.0f,  // bottom right
    // triangle two
     1.0f,  1.0f, 0.0f   // top right
    };
    // index data
    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 1, 3   // second triangle
    }; 
    // transform
    Vector3 position = {0.0f, 0.0f, 0.0f};
    Vector3 scale = {1.0f, 1.0f, 1.0f};
    Vector3 rotation = {0.0f, 0.0f, 0.0f};

    //material
    Color ambient = {0.1f, 0.1f, 0.1f, 1.0f};
   // Color diffuse = {0.0f, 0.0f, 1.0f, 1.0f};
    Color specular = {0.6f, 0.6f, 0.6f, 1.0f};
    float shininess = 32.0f;
    Material material = {ambient, diffuse, specular, shininess};

    createMesh(vertices,4,indices,6,&position,&scale,&rotation,&material,ui);
}

void initProgram(){
    initWindow();
    initECS();
}

int pollEvent(){
    return SDL_PollEvent(&globals.event);
}

void input() {
    // Process events
    while(pollEvent()) {
        
        if(globals.event.type == SDL_QUIT) {
            globals.running = 0;
        } 
        if(globals.event.type == SDL_KEYDOWN) {
            const char *key = SDL_GetKeyName(globals.event.key.keysym.sym);
            if(strcmp(key, "C") == 0) {
                glClearColor(randFloat(0.0,1.0),randFloat(0.0,1.0),randFloat(0.0,1.0), 1.0);
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
        }
        if(globals.event.type == SDL_WINDOWEVENT && globals.event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
            int w, h; 
            SDL_GetWindowSize(globals.window, &w, &h);
            printf("New Window size: %d x %d\n", w, h);
            // TODO: use this for reprojection later: float aspect = (float)w / (float)h;
            glViewport(0, 0, w / 2, h);
        }
    }
}

// Time variables
#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)
int last_frame_time = 0;

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
  
}

void render(){

    // Clear the entire window
    glClear(GL_COLOR_BUFFER_BIT);

    // Render without ui on wasm
    #ifdef __EMSCRIPTEN__
    setViewport(views[2]);
     for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1 && globals.entities[i].uiComponent->active == 0) {
                Color* diff = &globals.entities[i].materialComponent->diffuse;
                Color* amb = &globals.entities[i].materialComponent->ambient;
                Color* spec = &globals.entities[i].materialComponent->specular;
                float shin = globals.entities[i].materialComponent->shininess;
                renderMesh(globals.entities[i].meshComponent->gpuData,diff,amb,spec,shin);
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
                    setViewport(views[1]);
                    Color* diff = &globals.entities[i].materialComponent->diffuse;
                    Color* amb = &globals.entities[i].materialComponent->ambient;
                    Color* spec = &globals.entities[i].materialComponent->specular;
                    float shin = globals.entities[i].materialComponent->shininess;
                    renderMesh(globals.entities[i].meshComponent->gpuData,diff,amb,spec,shin);
                }else {
                    setViewport(views[0]);
                    Color* diff = &globals.entities[i].materialComponent->diffuse;
                    Color* amb = &globals.entities[i].materialComponent->ambient;
                    Color* spec = &globals.entities[i].materialComponent->specular;
                    float shin = globals.entities[i].materialComponent->shininess;
                    renderMesh(globals.entities[i].meshComponent->gpuData,diff,amb,spec,shin);
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

int main(int argc, char **argv) {
    // Initialize the random number generator
    srand((unsigned int)time(NULL));
    
    initProgram();
   
    initOpenGLWindow();
    
    Color blue = {0.0f, 0.0f, 1.0f, 1.0f};
    Color red = {1.0f, 0.0f, 0.0f, 1.0f};
    Color green = {0.0f, 1.0f, 0.0f, 1.0f};

    // 3d scene objects creation
    createRectangle(0,red);
    createTriangle(0, blue);
   

    // ui scene objects creation
    createRectangle(1, green);
  
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
    quit();
    printf("Non emscripten shutdown complete!\n");
    return 0;
    #endif
}
