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
#include "text.h"
#include "ecs.h"
#include "ecs-entity.h"
#include "ecs-systems.h"
#include "api.h"

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
    .mouseLeftButtonPressed=false,
    .mouseDoubleClick=false,
    .deselectCondition=false,
    .mouseDragged=false,

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
    .objDataCapacity=10,
    .lights={{0}},
    .lightsCount=0,
    .focusedEntityId=-1,
    .blinnMode=false,
    .gamma=false,
    .depthMapBuffer={0},
    .frameBuffer={0},
    .depthMap=0,
    .lightSpaceMatrix={{0}},
    .postProcessBuffer={0},
    .showDepthMap=false,
    .shadowWidth=1024,
    .shadowHeight=1024,

    // Cursor
    .cursorEntityId=-1,
    .cursorBlinkTime=0.5f,
    .cursorSelectionActive=false,
    .cursorDragStart=-1.0f,
    .cursorTextSelection={0,0}
    
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
void setViewportForDepthMapShadowRender(View view){
    Rectangle rect = convertViewRectangleToSDLCoordinates(view,globals.views.full.rect.height);
   
    // Set the viewport
    glViewport(0, 0, globals.shadowWidth, globals.shadowHeight);

     // Set the clear color
    glClearColor(view.clearColor.r, view.clearColor.g, view.clearColor.b, view.clearColor.a);

    // Clear the viewport
    glClear(GL_DEPTH_BUFFER_BIT);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

void createLightSpace(){
    mat4x4 lightProjection, lightView;
    float near_plane = 0.10f, far_plane = 30.0f;
    mat4x4_ortho(lightProjection, -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
  /*   printf(" globals.lights[0].transformComponent->position \n");
    printf("x %f ", globals.lights[0].transformComponent->position[0]);
    printf("y %f ", globals.lights[0].transformComponent->position[1]);
    printf("z %f ", globals.lights[0].transformComponent->position[2]); */
    vec3 dir = {
        globals.lights[0].lightComponent->direction[0], 
        globals.lights[0].lightComponent->direction[1], 
        globals.lights[0].lightComponent->direction[2]
    };
    vec3 pos = {
    globals.lights[0].transformComponent->position[0],
    globals.lights[0].transformComponent->position[1],
    globals.lights[0].transformComponent->position[2]
};

// Construct target position: pos + dir
vec3 target = {
    pos[0] + dir[0],
    pos[1] + dir[1],
    pos[2] + dir[2]
};
//target[0] = 0.0f;
//target[1] = 0.0f;
//target[2] = 0.0f;
vec3 tempLightPos;
tempLightPos[0] = -2.0f;
tempLightPos[1] = 4.0f;
tempLightPos[2] = -1.0f;

tempLightPos[0] = 0.01f;
tempLightPos[1] = 15.0f;
tempLightPos[2] = 0.0f;

    mat4x4_look_at(lightView, tempLightPos,  (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    mat4x4_mul(globals.lightSpaceMatrix, (const float (*)[4])lightProjection, (const float (*)[4])lightView);

    for(int i = 0; i < MAX_ENTITIES; i++){
        if(globals.entities[i].alive == 1 && globals.entities[i].lightComponent->active == 1 && globals.entities[i].lightComponent->type == DIRECTIONAL){
            
            globals.entities[i].lineComponent->end[0] = target[0];
            globals.entities[i].lineComponent->end[1] = target[1];
            globals.entities[i].lineComponent->end[2] = target[2];
         //   updateLine(globals.entities[i].lineComponent);

        }
    }
}

bool initShadowMap(){
   
   GLenum none = GL_NONE;
   // use 1K x 1K texture for shadow map
   int shadowMapTextureWidth = 32;
   int shadowMapTextureHeight = 32;
   glGenTextures ( 1, &globals.depthMap );
   glBindTexture ( GL_TEXTURE_2D, globals.depthMap);
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
   // set up hardware comparison
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,GL_COMPARE_REF_TO_TEXTURE );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC,GL_LEQUAL );
   glTexImage2D ( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16,shadowMapTextureWidth,shadowMapTextureHeight,
 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,
 NULL );
   glBindTexture ( GL_TEXTURE_2D, 0 );
   GLint defaultFramebuffer = 0;
   glGetIntegerv ( GL_FRAMEBUFFER_BINDING,
 &defaultFramebuffer );
   // set up fbo
   glGenFramebuffers ( 1, &globals.depthMapBuffer.FBO );
   glBindFramebuffer ( GL_FRAMEBUFFER, globals.depthMapBuffer.FBO );
   glDrawBuffers ( 1, &none );
   glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, globals.depthMap, 0 );

 glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, globals.depthMap);
   if ( GL_FRAMEBUFFER_COMPLETE !=
 glCheckFramebufferStatus ( GL_FRAMEBUFFER ) )
   {
 return false;
   }
   glBindFramebuffer ( GL_FRAMEBUFFER, defaultFramebuffer );
   return true;
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
                globals.blinnMode = 1;
            }
            if(strcmp(key, "U") == 0){
                globals.blinnMode = 0;
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

           
            
        }
        if (globals.event.type == SDL_MOUSEMOTION) {
            if(globals.mouseLeftButtonPressed){
                globals.mouseDragged = true;
            }

            int xpos =  globals.event.motion.x;
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
              //printf("Mouse moved to %f, %f\n", mouseX, mouseY);
           // SDLVector2 sdlMouseCoords = {xpos, ypos};
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
    //hoverAndClickSystem();
    textCursorSystem();
   // movementSystem();
    modelSystem();
   
}

void fboFromESBook(){
 GLuint framebuffer;
 GLuint depthRenderbuffer;
 GLuint texture;
 GLint texWidth = 256, texHeight = 256;
 GLint maxRenderbufferSize;
 glGetIntegerv ( GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
 // check if GL_MAX_RENDERBUFFER_SIZE is >= texWidth and texHeight
 if ( ( maxRenderbufferSize <= texWidth ) || ( maxRenderbufferSize <= texHeight ) ) 
{
    printf("error\n");
   // cannot use framebuffer objects, as we need to create
   // a depth buffer as a renderbuffer object
   // return with appropriate error 
}
 // generate the framebuffer, renderbuffer, and texture object names
 glGenFramebuffers ( 1, &framebuffer ); 
 glGenRenderbuffers ( 1, &depthRenderbuffer ); 
 glGenTextures ( 1, &texture );
 // bind texture and load the texture mip level 0
 // texels are RGB565
 // no texels need to be specified as we are going to draw into
 // the texture
 glBindTexture ( GL_TEXTURE_2D, texture );
 glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL );
 glTexParameteri ( GL_TEXTURE_2D,  GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
 glTexParameteri ( GL_TEXTURE_2D,  GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
 glTexParameteri ( GL_TEXTURE_2D,  GL_TEXTURE_MAG_FILTER, GL_LINEAR );
 glTexParameteri ( GL_TEXTURE_2D,  GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 // bind renderbuffer and create a 16-bit depth buffer 
 // width and height of renderbuffer = width and height of 
 // the texture
 glBindRenderbuffer ( GL_RENDERBUFFER, depthRenderbuffer ); 
 glRenderbufferStorage ( GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, texWidth, texHeight );
 // bind the framebuffer 
 glBindFramebuffer ( GL_FRAMEBUFFER, framebuffer );
 // specify texture as color attachment
 glFramebufferTexture2D ( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0 );
 // specify depth_renderbuffer as depth attachment 
 glFramebufferRenderbuffer ( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER, depthRenderbuffer);
 // check for framebuffer complete
 GLenum status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
if ( status == GL_FRAMEBUFFER_COMPLETE )
{
    printf("framebuffer complete\n");
    // render to texture using FBO
    // clear color and depth buffer
    glClearColor ( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // Load uniforms for vertex and fragment shaders
    // used to render to FBO. The vertex shader is the
    // ES 1.1 vertex shader described in Example 8-8 in
    // Chapter 8. The fragment shader outputs the color
    // computed by the vertex shader as fragment color and
    // is described in Example 1-2 in Chapter 1.
    // set_fbo_texture_shader_and_uniforms( );
    GpuData gpuData;
    setupMaterial(&gpuData,"shaders/esdepth_vert.glsl", "shaders/esdepth_frag.glsl");
   // glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
    // drawing commands to the framebuffer object draw_teapot();
     for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1) {
                if(globals.entities[i].uiComponent->active != 1){
                      // Set shader
                        glUseProgram(gpuData.shaderProgram);
                        
                        // retrieve the matrix uniform locations
                        unsigned int modelLoc = glGetUniformLocation(gpuData.shaderProgram, "model");
                        unsigned int viewLoc  = glGetUniformLocation(gpuData.shaderProgram, "view");
                        unsigned int projLoc  = glGetUniformLocation(gpuData.shaderProgram, "projection");

                        // pass them to the shaders 
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &globals.entities[i].transformComponent->transform[0][0]);
                        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &globals.views.main.camera->projection[0][0]);
                        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &globals.views.main.camera->view[0][0]);
                        
                       // glUniformMatrix4fv(glGetUniformLocation(globals.depthMapBuffer.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &globals.lightSpaceMatrix[0][0]);
                        
                        glBindVertexArray(globals.entities[i].meshComponent->gpuData->VAO);
                        glDrawArrays(globals.entities[i].meshComponent->gpuData->drawMode, 0, globals.entities[i].meshComponent->gpuData->vertexCount);
                    
                        glBindVertexArray(0);
                        glBindTexture(GL_TEXTURE_2D, 0);
                   // renderDepth(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,globals.views.main.camera,globals.entities[i].materialComponent);
                }
            }
        }
    }
    // render to window system-provided framebuffer 
    glBindFramebuffer ( GL_FRAMEBUFFER, 0 );
    // Use texture to draw to window system-provided framebuffer. 
    // We draw a quad that is the size of the viewport. 
    //
    // The vertex shader outputs the vertex position and texture 
    // coordinates passed as inputs.
    //
    // The fragment shader uses the texture coordinate to sample 
    // the texture and uses this as the per-fragment color value.
    float quadVertices[] = {
        // Positions    // Texture Coords
        -1.0f,  1.0f,    0.0f, 1.0f, // Top-left
        -1.0f, -1.0f,    0.0f, 0.0f, // Bottom-left
        1.0f,  1.0f,    1.0f, 1.0f, // Top-right
        1.0f, -1.0f,    1.0f, 0.0f  // Bottom-right
    };
    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture Coord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    GpuData screenShader;
    setupMaterial(&screenShader,"shaders/screentexture_vertex.glsl", "shaders/screentexture_fragment.glsl");

    // Clear the default framebuffer
glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Use the screen shader program
glUseProgram(screenShader.shaderProgram);

// Bind the framebuffer texture
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, texture); // 'texture' is the color attachment from the FBO
glUniform1i(glGetUniformLocation(screenShader.shaderProgram, "screenTexture"), 0);

// Render the quad
glBindVertexArray(quadVAO);
glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
glBindVertexArray(0);
   // set_screen_shader_and_uniforms ( ); 
   // draw_screen_quad ( ); 
}
 // clean up
 glDeleteRenderbuffers ( 1, &depthRenderbuffer );
 glDeleteFramebuffers ( 1, &framebuffer);
 glDeleteTextures ( 1, &texture );
 printf("done\n");
}

void test(){
    // FBO
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMap);
  //  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0, 0);
    //glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glDrawBuffers(GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1);

    GpuData depthMapBuffer;
    setupMaterial(&depthMapBuffer,"shaders/depth_vertex.glsl", "shaders/depth_fragment.glsl");

    mat4x4 lightProjection;
    mat4x4 lightView;
    mat4x4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 7.5f;
    mat4x4_ortho(lightProjection, -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    mat4x4_look_at(lightView, globals.lights[0].transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    mat4x4_mul(lightSpaceMatrix, (const float (*)[4])lightProjection, (const float (*)[4])lightView);

    glEnable(GL_DEPTH_TEST);

    glUseProgram(depthMapBuffer.shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(depthMapBuffer.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    printf("detph s\n");
    glClear(GL_DEPTH_BUFFER_BIT);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1) {
                if(globals.entities[i].uiComponent->active != 1){
                      // Set shader
                       // glUseProgram(globals.depthMapBuffer.shaderProgram);
                        
                        // retrieve the matrix uniform locations
                        unsigned int modelLoc = glGetUniformLocation(depthMapBuffer.shaderProgram, "model");
                    // unsigned int viewLoc  = glGetUniformLocation(globals.depthMapBuffer.shaderProgram, "view");

                        // pass them to the shaders 
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &globals.entities[i].transformComponent->transform[0][0]);
                        
                       // glUniformMatrix4fv(glGetUniformLocation(globals.depthMapBuffer.shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &globals.lightSpaceMatrix[0][0]);
                        
                        glBindVertexArray(globals.entities[i].meshComponent->gpuData->VAO);
                        glDrawArrays(globals.entities[i].meshComponent->gpuData->drawMode, 0, globals.entities[i].meshComponent->gpuData->vertexCount);
                    
                        glBindVertexArray(0);
                        glBindTexture(GL_TEXTURE_2D, 0);
                   // renderDepth(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,globals.views.main.camera,globals.entities[i].materialComponent);
                }
            }
        }
    }
   
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

    // Render depth map
  /*   if(!globals.depthMap){
       fboFromESBook();
       globals.depthMap = 1;
    } */
   // Rectangle rect = convertViewRectangleToSDLCoordinates(globals.views.full,globals.views.full.rect.height);
   // glViewport(rect.x, rect.y, rect.width, rect.height);
    
    // TODO: This only inits, no update if light moves and only calc on first light.
   // if(!globals.lightSpaceMatrix){
     //   printf("yes\n");
        createLightSpace();
   //    }
   /*  glBindFramebuffer(GL_FRAMEBUFFER, globals.depthMapBuffer.FBO);
    glViewport(0, 0, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);
    // Disable color writes
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
   // glActiveTexture(GL_TEXTURE0);
   // glBindTexture(GL_TEXTURE_2D, globals.depthMap);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1) {
                if(globals.entities[i].uiComponent->active != 1){
                    renderDepthMap(globals.entities[i].meshComponent->gpuData, globals.entities[i].transformComponent);
                }
            }
        }
    }
    // Re-enable color writes
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    
    renderFrameBuffer(); */
    // TODO: find a way to avoid having to iterate over all entities twice, once for ui and once for 3d objects.


   // clear depth buffer
   // glClear( GL_DEPTH_BUFFER_BIT );
   // disable color rendering; only write to depth buffer
   // glColorMask ( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
   // reduce shadow rendering artifact
   // glEnable ( GL_POLYGON_OFFSET_FILL );
   // glPolygonOffset( 4.0f, 100.0f );

   // Render depth map
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, globals.depthMapBuffer.FBO);
   setViewportForDepthMapShadowRender(globals.views.full);
   glEnable(GL_DEPTH_TEST);
   // Disable color buffer writes (since we're not drawing any color)
    //glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
   // setFontProjection(&globals.gpuFontData,globals.views.full);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].meshComponent->active == 1) {
                if(globals.entities[i].uiComponent->active != 1){
                    if(!globals.entities[i].materialComponent->isPostProcessMaterial){
                        //renderDepthMap(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent,globals.views.main.camera,globals.entities[i].materialComponent);
                        depthshadow_renderToDepthTexture(globals.entities[i].meshComponent->gpuData,globals.entities[i].transformComponent);
                    }
                }
            }
        }
    }

   // Enable color buffer writes again
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 

   // Render main view & 3d objects
   setViewportAndClear(globals.views.full);
   setFontProjection(&globals.gpuFontData,globals.views.full);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
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
   //printf("OpenGL context created!\n");
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

   globals.views.main.camera->position[1] = 35.0;
   globals.views.main.camera->position[2] = 35.0;
   printf("cam pos: %f %f %f \n",globals.views.main.camera->position[0],globals.views.main.camera->position[1],globals.views.main.camera->position[2]);

   // Assets
   initFont();
   //createFramebuffer();
   setupMaterial(&globals.depthMapBuffer, "shaders/depthMapBuffer_vert.glsl", "shaders/depthMapBuffer_frag.glsl");
  // setupDepthMap(&globals.depthMapBuffer);
 //  setupRenderBuffer(&globals.depthMapBuffer);
   //initShadowMap();
   //setupMaterial(&globals.depthMapBuffer,"shaders/depth_vertex.glsl", "shaders/depth_fragment.glsl");

    // New depth shadow steps:
    depthshadow_createFrameBuffer(&globals.depthMapBuffer);
    depthshadow_createDepthTexture(&globals.depthMapBuffer);
    depthshadow_configureFrameBuffer(&globals.depthMapBuffer);
   // depthshadow_renderToDepthTexture

  
 //  TextureData oldBricksTest = loadTexture("./Assets/oldbricks.jpg");
   TextureData containerTextureData = loadTexture("./Assets/container.jpg");
   GLuint containerMap = setupTexture(containerTextureData);
   TextureData containerTwoTextureData = loadTexture("./Assets/container2.png");
   GLuint containerTwoMap = setupTexture(containerTwoTextureData);
   TextureData containerTwoSpecTextureData = loadTexture("./Assets/container2_specular.png");
   GLuint containerTwoSpecularMap = setupTexture(containerTwoSpecTextureData);


    //ObjGroup* truck = obj_loadFile("./Assets/truck.obj"); // Not supported atm, need .obj group support.
   // ObjGroup* cornell_box = obj_loadFile("./Assets/cornell_box.obj");  
  /*    ObjGroup* bunny = obj_loadFile("./Assets/bunny2.obj");
    ObjGroup* plane = obj_loadFile("./Assets/plane.obj"); 
    ObjGroup* objExample = obj_loadFile("./Assets/Two_adjoining_squares_with_vertex_normals.obj");
    ObjGroup* sphere = obj_loadFile("./Assets/blender_sphere3.obj");
    ObjGroup* triangleVolumes = obj_loadFile("./Assets/triangle_volumes.obj");
    ObjGroup* teapot = obj_loadFile("./Assets/teapot.obj");*/
    ObjGroup* dragon = obj_loadFile("./Assets/dragon.obj");   
   ObjGroup* textured_objects = obj_loadFile("./Assets/textured_objects.obj");
 
    struct Material objectMaterial = {
    .active = 1,
    .name = "objectMaterial",
    .ambient = (Color){1.0f, 1.0f, 1.0f, 1.0f},  // NOT used,controlled from lightmaterial
    .diffuse = (Color){0.5f, 0.0f, 0.0f, 1.0f},  // used when diffuseMapOpacity lower than 1.0 (may not be true anymore)
    .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
    .shininess = 32.0f,                           // used
    .diffuseMap = containerTwoMap,               // used
    .diffuseMapOpacity = 1.0f,                  // used
    .specularMap = containerTwoSpecularMap,      // used
    };
   /*  struct Material textureUIMatExample = { // Saved as example of how to use a texture with ui element.
        .active = 1,
        .name = "textureUIMatExample",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = (Color){0.5f, 0.5f, 0.0f, 1.0f},  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 1.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    }; */
     struct Material depthMapMaterial = { // Saved as example of how to use a texture with ui element.
        .active = 1,
        .name = "depthMapMaterial",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = (Color){0.0f, 0.5f, 0.0f, 1.0f},  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 1.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
        .isPostProcessMaterial = true,
    }; 
   /*  struct Material textInputUiMat = { // Saved as example of how to use a texture with ui element.
        .active = 1,
        .name = "textInputUiMat",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = DARK_INPUT_FIELD,  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 0.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    }; */
  /*   struct Material flatColorUiDarkGrayMat = {
        .active = 1,
        .name = "flatColorUiDarkGrayMat",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = DARK_GRAY_COLOR,  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 0.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    }; */
   /*  struct Material flatColorUiGrayMat = {
        .active = 1,
        .name = "flatColorUiGrayMat",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = GRAY_COLOR,  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 0.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    }; */
    struct Material lightMaterial = {
        .active = 1,
        .name = "lightMaterial",
        .ambient = (Color){1.0f, 1.0f, 1.0f, 1.0f},  // used       <-- ambient strength & tint
        .diffuse = (Color){1.0f, 1.0f, 1.0f, 1.0f},  // used       <-- diffuse strength & tint
        .specular = (Color){1.0f,1.0f, 1.0f, 1.0f}, // used        <-- specular strength & tint
        .shininess = 32.0f,                         // NOT used
        .diffuseMap = containerMap,                  // NOT used
        .diffuseMapOpacity = 1.0f,                  // NOT used
        .specularMap = containerMap,                 // NOT used
    };

   
    // Main viewport objects (3d scene) x,y,z coords is a world space coordinate (not yet implemented?).
/*  createObject(&cornell_box->objData[0],(vec3){-5.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[1],(vec3){-5.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[2],(vec3){-5.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[3],(vec3){-5.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[4],(vec3){-5.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[5],(vec3){-5.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[6],(vec3){-5.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[7],(vec3){-5.0f, -5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});   */
 
   // createObject(&textured_objects->objData[0],(vec3){2.0f, 1.0f, -6.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});  
   // createObject(&textured_objects->objData[1],(vec3){2.0f, 1.0f, -6.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});   
   // createPoint((vec3){-5.0f, -5.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&plane->objData[0],(vec3){5.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&bunny->objData[0],(vec3){6.0f, 0.0f, 0.0f}, (vec3){10.0f, 10.0f, 10.0f}, (vec3){0.0f, 0.0f, 0.0f});   
    //createObject(VIEWPORT_MAIN,&truck->objData[0],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&objExample->objData[0],(vec3){5.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&sphere->objData[0],(vec3){3.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&triangleVolumes->objData[0],(vec3){4.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&teapot->objData[0],(vec3){0.0f, 0.0f, 0.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){-90.0f, 0.0f, 0.0f}); 
   createObject(&dragon->objData[0],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});     

    // lights
    createLight(lightMaterial,(vec3){0.0f,1.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){0.0f, -1.0f, 0.0f},DIRECTIONAL);
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
 //   createCube(objectMaterial,(vec3){2.0f, -0.0f, -0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
  
    // Frame buffer quad
    ui_createRectangle(depthMapMaterial, (vec3){0.0f, 0.0f,5.0f}, (vec3){800.0f, 600.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});

    debug_drawFrustum();

   // UI scene objects creation (2d scene) x,y coords where x = 0 is left and y = 0 is top and x,y is pixel positions. (controlled by the uiSystem)
   // Scale is in pixels, 100.0f is 100 pixels etc.
   // z position will be z-depth, much like in DOM in web.Use this to control draw order.
   // TODO: implement rotation, it is atm not affecting. 
 
    // UI Settings Panel
  /*   ui_createButton(flatColorUiGrayMat, (vec3){545.0f, 5.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Header",onButtonClick);
    ui_createTextInput(textInputUiMat, (vec3){565.0f, 65.0f, 1.0f}, (vec3){200.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "TextInput",onTextInputChange);
    ui_createRectangle(flatColorUiGrayMat, (vec3){545.0f, 55.0f, 0.0f}, (vec3){10.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    ui_createRectangle(flatColorUiDarkGrayMat, (vec3){555.0f, 55.0f, 1.0f}, (vec3){240.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    ui_createButton(flatColorUiGrayMat, (vec3){545.0f, 355.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Header2",onButtonClick);
    ui_createRectangle(flatColorUiGrayMat, (vec3){545.0f, 405.0f, 0.0f}, (vec3){10.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    ui_createRectangle(flatColorUiDarkGrayMat, (vec3){555.0f, 405.0f, 1.0f}, (vec3){240.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});  
    
    */

    //ui_slider(flatColorUiDarkGrayMat, (vec3){545.0f, 55.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Slider",onSliderChange);
    //ui_colorPicker(flatColorUiDarkGrayMat, (vec3){545.0f, 105.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "ColorPicker",onColorPickerChange);
    //ui_checkbox
    //ui_open_file (texture,obj-model,.fh-packfile,)


 //ui_createRectangle(uiMaterial, (vec3){765.0f, 5.0f, 0.0f}, (vec3){35.0f, 50.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f});

 // Textured button
 //ui_createButton(uiMaterial, (vec3){150.0f, 0.0f, 0.0f}, (vec3){150.0f, 50.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Rotate",onButtonClick);
  
   // TODO: create slider or input for ui using this
   /*  printf("materials-list (%d): \n",globals.materialsCount);
    for(int i = 0; i < globals.materialsCount; i++){
        printf("%s \n",globals.materials[i].name);
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
   }   */

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
void onTextInputChange(){
    printf("Text input changed!\n");
}

// -----------------------------------------------------
// ASSETS
// -----------------------------------------------------



    
