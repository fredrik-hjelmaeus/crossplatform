#ifndef UTILS_H   // If UTILS_H isn't defined...
#define UTILS_H   // Define it (with no particular value)

#include <stdio.h>
#include <stdlib.h>
#include "stb_image.h"
#include <string.h>
#include "types.h"

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


char* readFile(const char *filename);
int randInt(int rmin, int rmax);
float randFloat(float rmin, float rmax);
unsigned char* loadImage(const char* filename, int* width, int* height, int* nrChannels);
void handleFaceLine(char* line, int* vf, int* tf, int* vn, int* vfCount, int* tfCount, int* vnCount, int* faceLineCount);
void runTests();
void initMemoryArena(Arena* arena, size_t size);
TextureData loadTexture(char* path);

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


float deg2rad(float degrees);

ObjGroup* loadObjFile(const char* filepath);

int isPointInsideRect(Rectangle rect, vec2 point);
Rectangle convertViewRectangleToSDLCoordinates(View view,int windowHeight);
void convertUIcoordinateToWindowcoordinates(View view, TransformComponent* transformComponent, int windowHeight,int windowWidth,vec2 convertedPoint);
void captureFramebuffer(int width, int height, int drawCallsCounter);
#endif // End of the UTILS_H definition
