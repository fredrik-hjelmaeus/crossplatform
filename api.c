#include "render.h"
#include "api.h"
#include "app.h"
#include "opengl.h"
#include "ecs-entity.h"
#include "shadows.h"
#include "scene-objects.h"


void setTransformData(Entity* entity,vec3 position,vec3 scale,vec3 rotation){
    entity->transformComponent->active = 1;
    entity->transformComponent->position[0] = position[0];
    entity->transformComponent->position[1] = position[1];
    entity->transformComponent->position[2] = position[2];
    entity->transformComponent->scale[0] = scale[0];
    entity->transformComponent->scale[1] = scale[1];
    entity->transformComponent->scale[2] = scale[2];
    entity->transformComponent->rotation[0] = rotation[0] * M_PI / 180.0f; // convert to radians
    entity->transformComponent->rotation[1] = rotation[1] * M_PI / 180.0f;
    entity->transformComponent->rotation[2] = rotation[2] * M_PI / 180.0f;
    entity->transformComponent->modelNeedsUpdate = 1;
}
//----------------------------------------------------------------------------------------------//
/**
 * @brief Create a mesh
 * Main function to create a mesh. 
 *  - vertex data
 *  - index data
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
    ){
 
    const float *verts = geometry.vertices;
    size_t num_of_vertex = geometry.vertexCount;
    const unsigned int *indices = geometry.indices;
    size_t numIndicies = geometry.indexCount;

    entity->meshComponent->active = 1;

    // vertex data
    ASSERT(verts != NULL && num_of_vertex > 0, "Mesh needs vertices");
    Vertex *vertices = arena_Alloc(&app->meshArena, num_of_vertex * sizeof(*vertices));
    entity->meshComponent->vertices = vertices; // Own a copy; callers may use local arrays.
    
    int stride = 11;
    size_t vertexIndex = 0;

    // We have three types of vertex data input:
    // - one with color & indices VERT_COLOR_INDICIES
    // - one with color & no indicies VERT_COLOR
    // - one with no color & no indicies VERT
    if(vertexDataType == VERTS_ONEUV) {

        // vertices + texcoords but no indices and no color
        stride = 11;
        for(size_t i = 0; i < num_of_vertex * stride; i+=stride) {
            entity->meshComponent->vertices[vertexIndex].position[0] = verts[i];
            entity->meshComponent->vertices[vertexIndex].position[1] = verts[i + 1];
            entity->meshComponent->vertices[vertexIndex].position[2] = verts[i + 2];

            entity->meshComponent->vertices[vertexIndex].color[0] = verts[i + 3];
            entity->meshComponent->vertices[vertexIndex].color[1] = verts[i + 4];
            entity->meshComponent->vertices[vertexIndex].color[2] = verts[i + 5]; 

            entity->meshComponent->vertices[vertexIndex].texcoord[0] = verts[i + 6];
            entity->meshComponent->vertices[vertexIndex].texcoord[1] = verts[i + 7];

            entity->meshComponent->vertices[vertexIndex].normal[0] = verts[i + 8];
            entity->meshComponent->vertices[vertexIndex].normal[1] = verts[i + 9];
            entity->meshComponent->vertices[vertexIndex].normal[2] = verts[i + 10];

            vertexIndex++;
        } 

    }else if(vertexDataType == VERTS_COLOR_ONEUV_INDICIES){

        // indexed data with color
        for(size_t i = 0; i < num_of_vertex * stride; i+=stride) {
            entity->meshComponent->vertices[vertexIndex].position[0] = verts[i];
            entity->meshComponent->vertices[vertexIndex].position[1] = verts[i + 1];
            entity->meshComponent->vertices[vertexIndex].position[2] = verts[i + 2];
            entity->meshComponent->vertices[vertexIndex].color[0] = verts[i + 3]; 
            entity->meshComponent->vertices[vertexIndex].color[1] = verts[i + 4];
            entity->meshComponent->vertices[vertexIndex].color[2] = verts[i + 5];
            entity->meshComponent->vertices[vertexIndex].texcoord[0] = verts[i + 6];
            entity->meshComponent->vertices[vertexIndex].texcoord[1] = verts[i + 7];
            entity->meshComponent->vertices[vertexIndex].normal[0] = verts[i + 8];
            entity->meshComponent->vertices[vertexIndex].normal[1] = verts[i + 9];
            entity->meshComponent->vertices[vertexIndex].normal[2] = verts[i + 10];
            vertexIndex++;
        }
    }else if(vertexDataType == VERTS_COLOR_ONEUV){
         // not indexed data with color
        for(size_t i = 0; i < num_of_vertex * stride; i+=stride) {
             entity->meshComponent->vertices[vertexIndex].position[0] = verts[i];
            entity->meshComponent->vertices[vertexIndex].position[1] = verts[i + 1];
            entity->meshComponent->vertices[vertexIndex].position[2] = verts[i + 2];

            entity->meshComponent->vertices[vertexIndex].color[0] = verts[i + 3];
            entity->meshComponent->vertices[vertexIndex].color[1] = verts[i + 4];
            entity->meshComponent->vertices[vertexIndex].color[2] = verts[i + 5]; 

            entity->meshComponent->vertices[vertexIndex].texcoord[0] = verts[i + 6];
            entity->meshComponent->vertices[vertexIndex].texcoord[1] = verts[i + 7];

            entity->meshComponent->vertices[vertexIndex].normal[0] = verts[i + 8];
            entity->meshComponent->vertices[vertexIndex].normal[1] = verts[i + 9];
            entity->meshComponent->vertices[vertexIndex].normal[2] = verts[i + 10];

            vertexIndex++;
        }
    }else {
        printf("UNSUPPORTED vertexData");
        exit(1);
    }
    
    entity->meshComponent->vertexCount = num_of_vertex;
    entity->meshComponent->gpuData->vertexCount = num_of_vertex;

    // index data
    ASSERT(numIndicies == 0 || indices != NULL, "Indexed mesh needs indices");
    GLuint *indicies = numIndicies > 0
        ? arena_Alloc(&app->meshArena, (size_t)numIndicies * sizeof(*indicies))
        : NULL;
    entity->meshComponent->indices = indicies;
    for(size_t i = 0; i < numIndicies; i++) {
        entity->meshComponent->indices[i] = indices[i];
    }
    entity->meshComponent->indexCount = numIndicies;

    // transform data
    setTransformData(entity,position,scale,rotation);

    // material data
    entity->materialComponent->active = 1;
    entity->materialComponent->ambient = material->ambient;
    entity->materialComponent->diffuse = material->diffuse;
    entity->materialComponent->specular = material->specular;
    entity->materialComponent->shininess = material->shininess;
    entity->materialComponent->diffuseMap = material->diffuseMap;
    entity->materialComponent->diffuseMapOpacity = material->diffuseMapOpacity;
    entity->materialComponent->specularMap = material->specularMap;
    if(material->isPostProcessMaterial){
        entity->materialComponent->isPostProcessMaterial = true;
    }
    if(saveMaterial){
        entity->materialComponent->materialIndex = addMaterial(app, *material);
    }else {
        // TODO: temp fix for createObjects. The do not set shininess correctly atm.
        entity->materialComponent->shininess = 32.0f;
    }
    // TODO: Implement these when we need them.
   /*  if(!material->ambientMap){
        // Clear flag
        material->material_flags &= ~MATERIAL_AMBIENTMAP_ENABLED;
        // ALT: Toggle the flag: flags ^= FLAG_ENABLED;
    }
    if(!material->shininessMap){
        material->material_flags &= ~MATERIAL_SHININESSMAP_ENABLED;
    }
    if(!material->specularMap){
        material->material_flags &= ~MATERIAL_SPECULARMAP_ENABLED;
    } */
    if(material->diffuseMap == 0){ //|| material->diffuseMapOpacity == 0.0
        material->material_flags &= ~MATERIAL_DIFFUSEMAP_ENABLED; 
    }
    
    entity->materialComponent->material_flags = material->material_flags;
 
    entity->meshComponent->gpuData->drawMode = drawMode;
    
    setupMesh(  entity->meshComponent->vertices, 
                entity->meshComponent->vertexCount, 
                entity->meshComponent->indices, 
                entity->meshComponent->indexCount,
                entity->meshComponent->gpuData
                );
    
    // Shared shader selection happens in render(), based on the pass and entity type.
        //setupMaterial( entity->meshComponent->gpuData,"shaders/mesh_vertex.glsl", "shaders/mesh_fragment.glsl" );
        //setupMaterial(&renderState->postProcessBuffer, "shaders/mesh_vertex.glsl", "shaders/mesh_fragment.glsl");
}

// TODO: unfinished. Supposed to draw the frustum of the camera or light correctly.
void debug_drawFrustum(AppState *app)
{
     //float near_plane = -0.05f, far_plane = 300.0f;
    //mat4x4_ortho(lightProjection, -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    Entity* entity = addEntity(app, MODEL);
   // vec3 position;
    //vec3 scale;
    vec3 rotation;
/*     position[0] = 0.0f;
    position[1] = 0.0f;
    position[2] = 0.0f; */
   /*  scale[0] = 0.0f;
    scale[1] = 0.0f;
    scale[2] = 0.0f; */
    rotation[0] = 0.0f;
    rotation[1] = 0.0f;
    rotation[2] = 0.0f;
    
    // Near plane top
    entity->transformComponent->position[0] = 0;
    entity->transformComponent->position[1] = 0;
    entity->transformComponent->position[2] = 0;
    entity->transformComponent->scale[0] = 1.0f;
    entity->transformComponent->scale[1] = 1.0f;
    entity->transformComponent->scale[2] = 1.0f;
    entity->transformComponent->rotation[0] = rotation[0] * M_PI / 180.0f; // convert to radians
    entity->transformComponent->rotation[1] = rotation[1] * M_PI / 180.0f;
    entity->transformComponent->rotation[2] = rotation[2] * M_PI / 180.0f;
    entity->transformComponent->modelNeedsUpdate = 1;
  
    vec3 startt;
    vec3 endt;
    startt[0] = 0.0f;
    startt[1] = 10.0f;
    startt[2] = 0.0f;
    endt[0]   = 2.0f;
    endt[1]   = 10.0f;
    endt[2]   = 0.0f;
    Entity* entityHelper = addEntity(app, HELPER);
    vec3 starttPoints[2];
    vec3_dup(starttPoints[0], startt);
    vec3_dup(starttPoints[1], endt);
    scene_createLine(app, entityHelper, starttPoints, 2);
    
    // Near plane bottom
    Entity* entity4 = addEntity(app, MODEL);
    entity4->transformComponent->position[0] = 0;
    entity4->transformComponent->position[1] = 0;
    entity4->transformComponent->position[2] = 0;
    entity4->transformComponent->scale[0] = 1.0f;
    entity4->transformComponent->scale[1] = 1.0f;
    entity4->transformComponent->scale[2] = 1.0f;
    entity4->transformComponent->rotation[0] = rotation[0] * M_PI / 180.0f; // convert to radians
    entity4->transformComponent->rotation[1] = rotation[1] * M_PI / 180.0f;
    entity4->transformComponent->rotation[2] = rotation[2] * M_PI / 180.0f;
    entity4->transformComponent->modelNeedsUpdate = 1;
  
    vec3 startb;
    vec3 endb;
    startb[0] = 0.0f;
    startb[1] = 10.0f;
    startb[2] = 1.0f;
    endb[0]   = 2.0f;
    endb[1]   = 10.0f;
    endb[2]   = 1.0f;
    
    Entity* entityHelper2 = addEntity(app, HELPER);
    vec3 startbPoints[2];
    vec3_dup(startbPoints[0], startb);
    vec3_dup(startbPoints[1], endb);
    scene_createLine(app, entityHelper2, startbPoints, 2);
   
    // Near plane right
    Entity* entity2 = addEntity(app, MODEL);
    entity2->transformComponent->position[0] = 0;
    entity2->transformComponent->position[1] = 0;
    entity2->transformComponent->position[2] = 0;
    entity2->transformComponent->scale[0]    = 1.0f;
    entity2->transformComponent->scale[1]    = 1.0f;
    entity2->transformComponent->scale[2]    = 1.0f;
    entity2->transformComponent->rotation[0] = rotation[0] * M_PI / 180.0f; // convert to radians
    entity2->transformComponent->rotation[1] = rotation[1] * M_PI / 180.0f;
    entity2->transformComponent->rotation[2] = rotation[2] * M_PI / 180.0f;
    entity2->transformComponent->modelNeedsUpdate = 1;
    vec3 startr;
    vec3 endr;
    startr[0] = 1.0f;
    startr[1] = 10.0f;
    startr[2] = 0.0f;
    endr[0] = 1.0f;
    endr[1] = 10.0f;
    endr[2] = 2.0f;
    Entity* entityHelper3 = addEntity(app, HELPER);
    vec3 startrPoints[2];
    vec3_dup(startrPoints[0], startr);
    vec3_dup(startrPoints[1], endr);
    scene_createLine(app, entityHelper3, startrPoints, 2);

    // Near plane left
    Entity* entity3 = addEntity(app, MODEL);
    entity3->transformComponent->position[0] = 0;
    entity3->transformComponent->position[1] = 0;
    entity3->transformComponent->position[2] = 0;
    entity3->transformComponent->scale[0]    = 1.0f;
    entity3->transformComponent->scale[1]    = 1.0f;
    entity3->transformComponent->scale[2]    = 1.0f;
    entity3->transformComponent->rotation[0] = rotation[0] * M_PI / 180.0f; // convert to radians
    entity3->transformComponent->rotation[1] = rotation[1] * M_PI / 180.0f;
    entity3->transformComponent->rotation[2] = rotation[2] * M_PI / 180.0f;
    entity3->transformComponent->modelNeedsUpdate = 1;
    vec3 startl;
    vec3 endl;
    startl[0] = 0.0f;
    startl[1] = 10.0f;
    startl[2] = 0.0f;
    endl[0] = 0.0f;
    endl[1] = 10.0f;
    endl[2] = 2.0f;
    Entity* entityHelper4 = addEntity(app, HELPER);
    vec3 startlPoints[2];
    vec3_dup(startlPoints[0], startl);
    vec3_dup(startlPoints[1], endl);
    scene_createLine(app, entityHelper4, startlPoints, 2);
    
}

void createPoints(GLfloat* positions,int numPoints, Entity* entity){
    entity->pointComponent->active = 1;
    entity->pointComponent->color = (Color){1.0f,0.0f,1.0f,1.0f};
    entity->pointComponent->points = positions;
    entity->pointComponent->pointSize = 10.0f;
    entity->transformComponent->position[0] = positions[0];
    entity->transformComponent->position[1] = positions[1];
    entity->transformComponent->position[2] = positions[2];
    entity->transformComponent->scale[0] = 1.0f;
    entity->transformComponent->scale[1] = 1.0f;
    entity->transformComponent->scale[2] = 1.0f;
    entity->transformComponent->rotation[0] = 0.0f;
    entity->transformComponent->rotation[1] = 0.0f;
    entity->transformComponent->rotation[2] = 0.0f;
    entity->transformComponent->modelNeedsUpdate = 1;
    setupPoints(positions,numPoints,entity->pointComponent->gpuData);
}


