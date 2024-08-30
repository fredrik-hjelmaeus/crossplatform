#ifndef OPENGL_H   // If OPENGL_H isn't defined...
#define OPENGL_H   // Define it (with no particular value)

#include <stdio.h>
#include <stdlib.h>
#include "linmath.h"
#include "opengl_types.h"
#include "types.h"
#include "utils.h"


void renderMesh(GpuData* buffer,TransformComponent* transformComponent,Color* ambient,Color* diffuse, Color* specular,float shininess,GLuint diffuseMap,Camera* camera,bool useDiffuseMap);

void setupMaterial(GpuData* buffer);
void setupMesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, GpuData* buffer);
GLuint setupTexture(TextureData textureData);

void setupFontMaterial(GpuData* buffer,int width,int height);
void setupFontTextures(char* fontPath);
void setupFontMesh(GpuData *buffer);
void renderText(GpuData* buffer, char* text, float x, float y, float scale, Color color);

/**
 * @brief Generally called when view are switched
 * buffer is font gpu data
 */
void setFontProjection(GpuData *buffer,View view);
#endif // End of the OPENGL_H definition
