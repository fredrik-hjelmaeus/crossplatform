#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "types.h"

#define MAX_LIGHTSPACES 36

// GPU resources and settings for drawing the application.
typedef struct RenderState {
    SDL_Renderer* renderer;
    SDL_GLContext gl_context;
    GLenum overideDrawMode;
    int overrideDrawModeBool;
    bool drawBoundingBoxes;
    bool render;
    GpuData gpuFontData;
    bool culling;
    int drawCallsCounter;
    bool debugDrawCalls;
    bool blinnMode;
    bool gamma;
    GLuint depthMap;
    GLuint depthCubemap;
    mat4x4 lightSpaceMatrix[MAX_LIGHTSPACES];
    GpuData depthMapBuffer; // used to store depthmap shader
    GpuData frameBuffer; // used to store framebuffer shader
    GpuData postProcessBuffer; // used to store framebuffer shader
    bool showDepthMap;
    int shadowWidth;
    int shadowHeight;
    bool shadows;
} RenderState;

typedef struct AppState AppState;

RenderState renderDefaults(void);
void render(RenderState *renderState, AppState *app);

#endif // RENDER_H
