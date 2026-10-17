#ifndef OPENGL_H   // If OPENGL_H isn't defined...
#define OPENGL_H   // Define it (with no particular value)

typedef struct RenderState RenderState;


typedef struct AppState AppState;

#include <stdio.h>
#include <stdlib.h>
#include "linmath.h"
#include "opengl_types.h"
#include "types.h"
#include "utils.h"


void renderMesh(RenderState *renderState, AppState *app, GpuData* buffer,TransformComponent* transformComponent, Camera* camera,MaterialComponent* material);

void setupMaterial(GpuData* buffer,const char* vertexPath,const char* fragmentPath);
void setupMesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, GpuData* buffer);
GLuint setupTexture(RenderState *renderState, TextureData textureData);

void setupFontTextures(AppState *app, char* fontPath,int fontSize);
void setupFontMesh(GpuData *buffer);
void renderText(AppState *app, GpuData *buffer, char *text, float x, float y, float scale, Color color);
void renderLine(GpuData* buffer,TransformComponent* transformComponent, Camera* camera,Color lineColor);
void renderPoints(GpuData* buffer, TransformComponent* transformComponent, Camera* camera, Color pointColor,float pointSize);
void setupLine(GLfloat* lines, int lineCount, GpuData* buffer);
void updateLine(LineComponent* lineComponent);
void setupPoints(GLfloat* positions,int numPoints, GpuData* buffer);

// Shadow maps
void depthshadow_createFrameBuffer(GpuData* buffer);
void depthshadow_createDepthTexture(RenderState *renderState);
void depthshadow_createDepthCubemap(RenderState *renderState);
void depthshadow_setViewportForDepthMapShadowRender(RenderState *renderState, View view);
void depthshadow_configureFrameBuffer(GpuData* buffer, GLenum textureTarget, GLuint depthMap);
void depthshadow_renderToDepthTexture(RenderState *renderState, AppState *app, GpuData *buffer,TransformComponent *transformComponent);
/**
 * @brief Generally called when view are switched
 * buffer is font gpu data
 */
void setFontProjection(GpuData *buffer,View view);
#endif // End of the OPENGL_H definition
