#ifndef OPENGL_H   // If UTILS_H isn't defined...
#define OPENGL_H   // Define it (with no particular value)

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include "linmath.h"

// wasm/OpenGL ES 3.0
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <GLES3/gl3.h>

#elif __APPLE__
#include <OpenGL/gl3.h> // works but deprecated

#elif __linux__
#include <GLES3/gl3.h>

#elif _WIN32
// TBD
//#include <GL/gl3.h>
//#include <GL/glew.h>
#endif
#include "types.h"


void renderMesh(GpuData* buffer,Color* ambient,Color* diffuse, Color* specular,float shininess,GLuint diffuseMap);
void setupMaterial(GpuData* buffer);
void setupMesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, GpuData* buffer);
GLuint setupTexture(TextureData textureData);

#endif // End of the OPENGL_H definition

