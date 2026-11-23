#ifndef SCENE_OBJECTS_H
#define SCENE_OBJECTS_H

#include "types.h"

typedef struct AppState AppState;
typedef struct RenderState RenderState;

void scene_createPlane(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation);

/**
 * @brief Create a object. Used together with obj-load/parse. Expects data from obj-parser to be of type ObjData.
 * Create a object mesh
 * @param diffuse - color of the rectangle
*/
void scene_createObject(AppState *app, ObjData* obj,vec3 position,vec3 scale,vec3 rotation);

/**
 * @brief Create a light
 * Create a light source in the scene.
 * @return int - Entity id
 */
int scene_createLight(RenderState *renderState, AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation,vec3 direction,LightType type);

/**
 * @brief Create a Cube
 * Create a Cube mesh
 * @param diffuse - color of the cube
*/
void scene_createCube(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation);

/**
 * @brief Create a line segment between two points
 * Copies the points into storage owned by the line component.
 * @param app - needed for camera access
 * @param entity - entity to add line to
 * @param position - start position
 * @param endPosition - end position
 */
void scene_createLine(AppState *app, Entity *entity, vec3* points, int pointCount);

void scene_drawLine(AppState *app,vec3* points, int pointCount);

#endif
