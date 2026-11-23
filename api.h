#ifndef API_H
#define API_H

typedef struct RenderState RenderState;
typedef struct AppState AppState;

#include "event.h"
#include "types.h"
#include "geometry.h"

//------
void createPoints(GLfloat* positions,int numPoints, Entity* entity);

/**
 * @brief Create a mesh
 * Main function to create a mesh. 
 *  - geometry data
 *  - transform data
 *  - material data
*/
void scene_createMesh(
    AppState *app, 
    Geometry geometry,
    vec3 position,
    vec3 scale,
    vec3 rotation,
    Material* material,
    GLenum drawMode,
    VertexDataType vertexDataType,
    Entity* entity,
    bool saveMaterial // save material to global list
);


void debug_drawFrustum(AppState *app);

void setTransformData(Entity* entity,vec3 position,vec3 scale,vec3 rotation);

#endif // API_H
