
#include "app.h"
#include "geometry.h"
#include "shadows.h"
#include "ecs-entity.h"
#include "api.h"
#include "scene-objects.h"
#include "utils.h"
#include "opengl.h"

void scene_createPlane(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation)
{
    // Previous local plane data, retained for reference.
#if 0
    // vertex data
    GLfloat vertices[] = {
      // Positions          // Colors           // Texture Coords    // Normals
      -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  0.0f, 0.0f, 1.0f,    // Bottom-left
      0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,  0.0f, 0.0f, 1.0f,    // Bottom-right
      0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,  0.0f, 0.0f, 1.0f,    // Top-right
      0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,  0.0f, 0.0f, 1.0f,    // Top-right
      -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,  0.0f, 0.0f, 1.0f,    // Top-left
      -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,  0.0f, 0.0f, 1.0f,    // Bottom-left
    };
    // vertex data converted with help of chatgpt, might be wrong so saved above correct data
    /* Vertex vertices[] = {
        // Position              // Color             // TexCoord  // Normal
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Bottom-left
        {{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Bottom-right
        {{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}, // Top-right
        {{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}, // Top-right
        {{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}, // Top-left
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Bottom-left
    }; */

    // index data NOT USED ATM
    GLuint indices[] = {
    0, 1, 2, // First triangle
    2, 3, 0  // Second triangle
    };

#endif

    material.diffuseMapOpacity = 1.0;
    material.material_flags |= MATERIAL_DIFFUSEMAP_ENABLED;
   
    Entity* entity = addEntity(app, MODEL);

    scene_createMesh(app, geometry_plane,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
}

/**
 * @brief Create a Cube
 * Create a Cube mesh
 * @param diffuse - color of the cube
*/
void scene_createCube(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation)
{
  Entity* entity = addEntity(app, MODEL);

  material.diffuseMapOpacity = 1.0;
  material.material_flags |= MATERIAL_DIFFUSEMAP_ENABLED;
  
  scene_createMesh(app, geometry_cube,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
}

/**
 * @brief Create a line segment between two points
 * @param entity - entity to add line to
 * @param points - points
 * @param pointCount
 */
void scene_createLine(AppState *app, Entity *entity, vec3* points,int pointCount)
{
    ASSERT(points != NULL && pointCount >= 2, "Line needs at least two points");

    vec3 *ownedPoints = arena_Alloc(&app->meshArena,(size_t)pointCount * sizeof(*ownedPoints));

    for (int i = 0; i < pointCount; i++) {
        vec3_dup(ownedPoints[i], points[i]);
    }
    entity->lineComponent->active = 1;
    entity->lineComponent->points = ownedPoints;
    entity->transformComponent->active = 1;
    entity->transformComponent->modelNeedsUpdate = 1;
    entity->lineComponent->pointCount = pointCount;
    entity->lineComponent->color = (Color){1.0f,1.0f,0.0f,1.0f};
    entity->lineComponent->width = 60.0f;

    setupLine(geometry_rectangle, entity->lineComponent);
    // Segment and bevel programs are shared and selected during rendering.
}

void scene_drawLine(AppState *app,vec3* points, int pointCount)
{
    ASSERT(points != NULL && pointCount >= 2, "Line needs at least two points");
    Entity* entity = addEntity(app, MODEL);
    setTransformData(entity, (vec3){0, 0, 0}, (vec3){1, 1, 1}, (vec3){0, 0, 0});
    scene_createLine(app, entity, points, pointCount);
}

/**
 * @brief Create a light
 * Create a light source in the scene.
 * @return {int} Entity id
 */
int scene_createLight(
    RenderState *renderState, 
    AppState *app, 
    Material material,
    vec3 position,
    vec3 scale,
    vec3 rotation,
    vec3 direction,
    LightType type
)
{
  
    // cutOff
    float degrees = 12.5f;
    float radians = DEG_TO_RAD(degrees);
    float cosine = cosf(radians);

    // outerCutOff
    float degreesOC = 17.5f;
    float radiansOC = DEG_TO_RAD(degreesOC);
    float cosineOC = cosf(radiansOC);

    Entity* entity = addEntity(app, MODEL);
    entity->lightComponent->active = 1;
    entity->lightComponent->direction[0] = direction[0];
    entity->lightComponent->direction[1] = direction[1];
    entity->lightComponent->direction[2] = direction[2];
    entity->lightComponent->intensity = 1.0f;
    entity->lightComponent->diffuse = material.diffuse;
    entity->lightComponent->specular = material.specular;
    entity->lightComponent->ambient = material.ambient;
    entity->lightComponent->constant = 1.0f;
    entity->lightComponent->linear = 0.09f;
    entity->lightComponent->quadratic = 0.032f;
    entity->lightComponent->cutOff = cosine;
    entity->lightComponent->outerCutOff = cosineOC;
    entity->lightComponent->type = type;
    entity->lightComponent->castShadows = true;

    setTransformData(entity, position, scale, rotation);
    
    // TODO: This is a temporary solution, need to implement a better way to handle lights.
    ASSERT(app->lightsCount < MAX_LIGHTS, "Too many lights");
    app->lights[app->lightsCount].entityId = entity->id;
    app->lights[app->lightsCount].lightType = type;
    
    createDirectionalLightSpace(renderState, app, app->lightsCount);
    app->lightsCount++;

    if(type == DIRECTIONAL){
        vec3 endPosition;
        vec3 normalized_direction;
        vec3_norm(normalized_direction,direction);
        float scalar = 15.0f;
        vec3 scaled_direction;
        vec3_scale(scaled_direction, normalized_direction, scalar);
        
        // multiply normalized direction with a float here
        vec3_add(endPosition, scaled_direction, position);
        
        vec3 localStartPosition = {0.0f, 0.0f, 0.0f};
        Entity* entityHelper = addEntity(app, HELPER);
        setTransformData(entityHelper, position, (vec3){1, 1, 1}, (vec3){0, 0, 0});

        vec3 points[2];
        vec3_dup(points[0], localStartPosition);
        vec3_dup(points[1], scaled_direction);
        scene_createLine(app, entityHelper, points, 2);
        entityHelper->lineComponent->width = 3.0f; // Pixels in the active viewport.
        
        entityHelper->helperComponent->active = 1;
        entityHelper->helperComponent->entityRefId = entity->id;
        entityHelper->helperComponent->entityRefGeneration = entity->generation;
    }
    
    return entity->id;
}

/**
 * @brief Create a object. 
 * Used together with obj-load/parse. 
 * Expects data from obj-parser to be of type ObjData.
 * Create a object mesh
 * @param diffuse - color of the rectangle
*/
void scene_createObject(AppState *app, ObjData* obj,vec3 position,vec3 scale,vec3 rotation)
{
    Entity* entity = addEntity(app, MODEL);

    Geometry geometry = {
        .vertices = (const float *)obj->vertexData,
        .vertexCount = obj->num_of_vertices,
        .indices = NULL,
        .indexCount = 0
    };
    
    scene_createMesh(app, geometry,position,scale,rotation,&app->materials[obj->materialIndex],GL_TRIANGLES,VERTS_COLOR_ONEUV,entity,false);
}
