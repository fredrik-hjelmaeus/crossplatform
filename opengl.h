#ifndef OPENGL_H   // If OPENGL_H isn't defined...
#define OPENGL_H   // Define it (with no particular value)

#include <stdio.h>
#include <stdlib.h>
#include "linmath.h"
#include "opengl_types.h"
#include "types.h"
#include "utils.h"


void renderMesh(GpuData* buffer,Color* ambient,Color* diffuse, Color* specular,float shininess,GLuint diffuseMap);
void setupMaterial(GpuData* buffer);
void setupMesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, GpuData* buffer);
GLuint setupTexture(TextureData textureData);

#endif // End of the OPENGL_H definition

