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

char* readFile(const char *filename);
int randInt(int rmin, int rmax);
float randFloat(float rmin, float rmax);
unsigned char* loadImage(const char* filename, int* width, int* height, int* nrChannels);

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
/* typedef struct ObjFile {
    Vertex* vertices;
    size_t vertexCount;
    unsigned int* indices;
    size_t indexCount;
} ObjFile; */
ObjData loadObjFile(const char* filepath);

int isPointInsideRect(Rectangle rect, vec2 point);
Rectangle convertViewRectangleToSDLCoordinates(View view,int windowHeight);
void convertUIcoordinateToWindowcoordinates(View view, TransformComponent* transformComponent, int windowHeight,int windowWidth,vec2 convertedPoint);

#endif // End of the UTILS_H definition


