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



// Max entities constant
#define MAX_ENTITIES 100

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
    meshComponent->gpuBuffer = (GpuBuffer*)malloc(sizeof(GpuBuffer));
    if(meshComponent->gpuBuffer == NULL) {
        printf("Failed to allocate memory for gpuBuffer\n");
        exit(1);
    }
}

void initializeMaterialComponent(MaterialComponent* materialComponent){
    materialComponent->active = 0;
    materialComponent->ambient.r = 0.0f;
    materialComponent->ambient.g = 0.0f;
    materialComponent->ambient.b = 0.20f;
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

void initECS(){
    // Allocate memory for MAX_ENTITIES Components structs
    TransformComponent* transformComponents = allocateComponentMemory(sizeof(TransformComponent), "transform");
    GroupComponent* groupComponents = allocateComponentMemory(sizeof(GroupComponent), "group");
    MeshComponent* meshComponents = allocateComponentMemory(sizeof(MeshComponent), "mesh");
    MaterialComponent* materialComponents = allocateComponentMemory(sizeof(MaterialComponent), "material");

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
    }

    globals.entities = entities;
}


Entity* addEntity(enum Tag tag){
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 0) {
            globals.entities[i].alive = 1;
            globals.entities[i].tag = tag;

            //printf("Entity with id %d added!\n", i);
            return &globals.entities[i];
        }
    }
    // if we get here we are out of entities and need to increase the MAX_ENTITIES constant 
    // for now, we just exit the program
    printf("Out of entities\n");
    exit(1);
}



void createMesh(){
    
     // Set up vertex data and buffers
   const float verts[] = {
         0.0f,  0.5f, 0.0f,  // top
        -0.5f, -0.5f, 0.0f,  // bottom left
         0.5f, -0.5f, 0.0f   // bottom right
    };

 
    const unsigned int indices[] = {
        0, 1, 2,  // first triangle
        1, 2, 3   // second triangle
    }; 

    Entity* entity = addEntity(MODEL);
    
    entity->meshComponent->active = 1;

    // vertex data
    entity->meshComponent->vertices = (Vertex*)malloc(3 * sizeof(Vertex));
     if (entity->meshComponent->vertices == NULL) {
        printf("Failed to allocate memory for vertices\n");
        exit(1);
    }
    for(int i = 0; i < 3; i++) {
        entity->meshComponent->vertices[i].position.x = verts[i * 3];
        entity->meshComponent->vertices[i].position.y = verts[i * 3 + 1];
        entity->meshComponent->vertices[i].position.z = verts[i * 3 + 2];
    }
    entity->meshComponent->vertexCount = 3;

    // index data
    entity->meshComponent->indices = (unsigned int*)malloc(6 * sizeof(unsigned int));
    if(entity->meshComponent->indices == NULL) {
        printf("Failed to allocate memory for indices\n");
        exit(1);
    }
    for(int i = 0; i < 6; i++) {
        entity->meshComponent->indices[i] = indices[i];
    }
    entity->meshComponent->indexCount = 6;

    // transform data
    entity->transformComponent->active = 1;
    entity->transformComponent->position.x = 0.0f;
    entity->transformComponent->position.y = 0.0f;
    entity->transformComponent->position.z = 0.0f;
    entity->transformComponent->scale.x = 1.0f;
    entity->transformComponent->scale.y = 1.0f;
    entity->transformComponent->scale.z = 1.0f;
    entity->transformComponent->rotation.x = 0.0f;
    entity->transformComponent->rotation.y = 0.0f;
    entity->transformComponent->rotation.z = 0.0f;
    entity->transformComponent->isDirty = 1;

    // material data
    entity->materialComponent->active = 1;
    entity->materialComponent->ambient.r = 0.1f;
    entity->materialComponent->ambient.g = 0.1f;
    entity->materialComponent->ambient.b = 0.1f;
    entity->materialComponent->ambient.a = 1.0f;
    entity->materialComponent->diffuse.r = 0.6f;
    entity->materialComponent->diffuse.g = 0.6f;
    entity->materialComponent->diffuse.b = 0.6f;
    entity->materialComponent->diffuse.a = 1.0f;
    entity->materialComponent->specular.r = 0.6f;
    entity->materialComponent->specular.g = 0.6f;
    entity->materialComponent->specular.b = 0.6f;
    entity->materialComponent->specular.a = 1.0f;
    entity->materialComponent->shininess = 32.0f;

    setupMesh(  entity->meshComponent->vertices, 
                entity->meshComponent->vertexCount, 
                entity->meshComponent->indices, 
                entity->meshComponent->indexCount,
                entity->meshComponent->gpuBuffer );

    setupMaterial( entity->meshComponent->gpuBuffer ); 
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
            if(globals.entities[i].meshComponent->active == 1) {
                renderMesh(globals.entities[i].meshComponent->gpuBuffer);
            }
        }
    }
    #else

    // Native
    // Render 3d scene 
    setViewport(views[0]);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1) {
                renderMesh(globals.entities[i].meshComponent->gpuBuffer);
            }
        }
    }
   
    // Render ui scene
    setViewport(views[1]);
    render_ui();
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
    
    // 3d scene objects creation
    createMesh();

    // ui scene objects creation
    setup_ui();
  
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
