#ifndef API_H
#define API_H

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
void createMesh(
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
void createObject(ObjData* obj,vec3 position,vec3 scale,vec3 rotation);
/**
 * @brief Create a light
 * Create a light source in the scene.
 * 
 */
void createLight(Material material,vec3 position,vec3 scale,vec3 rotation,vec3 direction,LightType type);
/**
 * @brief Create a Cube
 * Create a Cube mesh
 * @param diffuse - color of the cube
*/
void createCube(Material material,vec3 position,vec3 scale,vec3 rotation);
void createPlane(Material material,vec3 position,vec3 scale,vec3 rotation);
/**
 * @brief Create a line segment between two points
 * @param position - start position
 * @param endPosition - end position
 */
void createLine(vec3 position, vec3 endPosition,Entity* entity);

void debug_drawFrustum();

//-------
// UI API
//-------
/**
 * @brief Create a panel
 * Create a panel in ui. No visibility, only a bounding box.
 * @param material - material for bounding box
*/
Entity* ui_createPanel(Material material,vec3 position,vec3 scale,vec3 rotation,Entity* parent);
/**
 * @brief Create a rectangle
 * Create a rectangle mesh
*/
int ui_createRectangle(Material material,vec3 position,vec3 scale,vec3 rotation,Entity* parent);
/**
 * @brief Create a button
 * Create a button mesh in ui
*/
void ui_createButton(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,ButtonCallback onClick,Entity* parent);
/**
 * @brief Create a input field
 * Create a input mesh in ui
*/
void ui_createTextInput(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,OnChangeCallback onChange,Entity* parent);

#endif // API_H
