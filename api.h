#ifndef API_H
#define API_H

typedef struct RenderState RenderState;


typedef struct AppState AppState;

#include "event.h"
#include "types.h"

//------
void createPoints(GLfloat* positions,int numPoints, Entity* entity);
/**
 * @brief Create a mesh
 * Main function to create a mesh. 
 *  - vertex data
 *  - index data
 *  - transform data
 *  - material data
*/
void createMesh(AppState *app, 
    GLfloat* verts,
    GLuint num_of_vertex, 
    GLuint* indices, // atm plug in some dummy-data if not used.
    GLuint numIndicies, // atm just set to 0 if not used.
    vec3 position,
    vec3 scale,
    vec3 rotation,
    Material* material,
    GLenum drawMode,
    VertexDataType vertexDataType,
    Entity* entity,
    bool saveMaterial // save material to global list
    );
//-------
// 3D API
//-------
/**
 * @brief Create a object. Used together with obj-load/parse. Expects data from obj-parser to be of type ObjData.
 * Create a object mesh
 * @param diffuse - color of the rectangle
*/
void createObject(AppState *app, ObjData* obj,vec3 position,vec3 scale,vec3 rotation);
/**
 * @brief Create a light
 * Create a light source in the scene.
 * @return int - Entity id
 */
int createLight(RenderState *renderState, AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation,vec3 direction,LightType type);
/**
 * @brief Create a Cube
 * Create a Cube mesh
 * @param diffuse - color of the cube
*/
void createCube(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation);
void createPlane(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation);
/**
 * @brief Create a line segment between two points
 * @param position - start position
 * @param endPosition - end position
 */
void createLine(vec3 position, vec3 endPosition,Entity* entity);

void debug_drawFrustum(AppState *app);

void setTransformData(Entity* entity,vec3 position,vec3 scale,vec3 rotation);






#endif // API_H
