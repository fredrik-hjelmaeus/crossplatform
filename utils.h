#ifndef UTILS_H   // If UTILS_H isn't defined...
#define UTILS_H   // Define it (with no particular value)

#include <stdio.h>
#include <stdlib.h>
#include "stb_image.h"
#include <string.h>
#include "types.h"
#include <SDL2/SDL.h>

// Text output colors
#define TEXT_COLOR_ERROR     "\x1b[31m"
#define TEXT_COLOR_WARNING   "\x1b[33m"
#define TEXT_COLOR_RESET     "\x1b[0m"

#define TEXT_COLOR_YELLOW "\033[1;33m"
#define TEXT_COLOR_BLUE "\033[1;34m"
#define TEXT_COLOR_GREEN "\033[1;32m"
#define TEXT_COLOR_RED "\033[1;31m"
#define TEXT_COLOR_CYAN "\033[1;36m"
#define TEXT_COLOR_MAGENTA "\033[1;35m"
#define TEXT_COLOR_ORANGE "\033[1;33m"
#define TEXT_COLOR hexToColor("#bbbcc4")

// Common colors
#define GRAY_COLOR hexToColor("#3d3f45")
#define DARK_GRAY_COLOR hexToColor("#2f3137")
#define DARK_INPUT_FIELD hexToColor("#26272C")
#define BOUNDINGBOX_COLOR hexToColor("#f1c40f")

SDLVector2 convertUIToSDL(UIVector2 v,int screenWidth,int screenHeight);
UIVector2 convertSDLToUI(SDLVector2 v, int screenWidth, int screenHeight);
float absValue(float value);
Color hexToColor(const char* hex);
char* readFile(const char *filename);
unsigned char* loadImage(const char* filename, int* width, int* height, int* nrChannels);
TextureData loadTexture(char* path);
int randInt(int rmin, int rmax);
float randFloat(float rmin, float rmax);
bool isPointInsideBoundingBox(BoundingBox bb, vec2 point);
bool isPointInsideRect(Rectangle rect, vec2 point);
Rectangle convertViewRectangleToSDLCoordinates(View view,int windowHeight);
void convertUIcoordinateToWindowcoordinates(View view, TransformComponent* transformComponent, int windowHeight,int windowWidth,vec2 convertedPoint);
void captureDrawCalls(int width, int height, int drawCallsCounter);
float deg2rad(float degrees);
GLuint setupTexture(TextureData textureData);
void changeCursor(SDL_SystemCursor cursorType); 

// Memory
void arena_initMemory(Arena* arena, size_t size);
void* arena_Alloc(Arena* arena, size_t size);
void arena_reset(Arena* arena); // Not evaluated/used yet, do this before using.
void arena_free(Arena* arena);  // Not evaluated/used yet, do this before using.

// Utility macros
#define CHECK_SDL_ERROR(test, message) \
    do { \
        if((test)) { \
            fprintf(stderr, "%s\n", (message)); \
            exit(1); \
        } \
    } while(0)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define DEG_TO_RAD(degrees) ((degrees) * (M_PI / 180.0f))

// Materials
int getMaterialByName(const char* name);
int addMaterial(Material material);
Material* getMaterial(int index);

// Parse obj files
void obj_runTests();
void obj_handleFaceLine(char* line, int* vf, int* tf, int* vn, int* vfCount, int* tfCount, int* vnCount, int* faceLineCount);
char* obj_handleFilePath(const char* filepath);
ObjGroup* obj_loadFile(const char* filepath);
void obj_parseMaterial(const char* filepath);
bool obj_processTextureMap(char* mtlLine,const char* mapType,GLuint* map);

// Math
void vec3_subtract(vec3 a, vec3 b, vec3* result);
float vec3_length(vec3 v);

float magnitude(vec3 v);
float direction(vec3 v);
float elevation(vec3 v);
void toCartesianXYZ(float magnitude,float direction,float elevation, vec3* result);

#endif // End of the UTILS_H definition
