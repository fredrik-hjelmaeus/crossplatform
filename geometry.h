#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stddef.h>

typedef struct Geometry {
    const float *vertices;
    size_t vertexCount;

    const unsigned int *indices;
    size_t indexCount;
} Geometry;

extern const Geometry geometry_cube;
extern const Geometry geometry_rectangle;
extern const Geometry geometry_rect_3d;
extern const Geometry geometry_rect_outline;
extern const Geometry geometry_plane;

#endif
