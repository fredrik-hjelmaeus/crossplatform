#include "api.h"
#include "globals.h"
#include "opengl.h"
#include "ecs-entity.h"

//----------------------------------------------------------------------------------------------//
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
    ){
 
    entity->meshComponent->active = 1;

    // vertex data
    entity->meshComponent->vertices = verts;
    
    int stride = 11;
    int vertexIndex = 0;

    // We have three types of vertex data input:
    // - one with color & indices VERT_COLOR_INDICIES
    // - one with color & no indicies VERT_COLOR
    // - one with no color & no indicies VERT
    if(numIndicies == 0 && vertexDataType == VERTS_ONEUV) {

        // vertices + texcoords but no indices and no color
        stride = 11;
        for(int i = 0; i < num_of_vertex * stride; i+=stride) {
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
        for(int i = 0; i < num_of_vertex * stride; i+=stride) {
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
        for(int i = 0; i < num_of_vertex * stride; i+=stride) {
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
    entity->meshComponent->indices = (GLuint*)malloc(numIndicies * sizeof(GLuint));
    if(entity->meshComponent->indices == NULL) {
        printf("Failed to allocate memory for indices\n");
        exit(1);
    }
    for(int i = 0; i < numIndicies; i++) {
        entity->meshComponent->indices[i] = indices[i];
    }
    entity->meshComponent->indexCount = numIndicies;

   /*  if(numIndicies == 0){
        printf("not using indicies\n");
    } */

    // transform data
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

    // material data
    entity->materialComponent->active = 1;
    entity->materialComponent->ambient = material->ambient;
    entity->materialComponent->diffuse = material->diffuse;
    entity->materialComponent->specular = material->specular;
    entity->materialComponent->shininess = material->shininess;
    entity->materialComponent->diffuseMap = material->diffuseMap;
    entity->materialComponent->diffuseMapOpacity = material->diffuseMapOpacity;
    entity->materialComponent->specularMap = material->specularMap;
    if(saveMaterial){
        entity->materialComponent->materialIndex = addMaterial(*material);
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
    if(material->diffuseMap == 0){
        material->material_flags &= ~MATERIAL_DIFFUSEMAP_ENABLED; 
        printf("material->material_flags & MATERIAL_DIFFUSEMAP_ENABLED %d \n",material->material_flags & MATERIAL_DIFFUSEMAP_ENABLED);
    }
    
    entity->materialComponent->material_flags = material->material_flags;
 
    entity->meshComponent->gpuData->drawMode = drawMode;
    
    setupMesh(  entity->meshComponent->vertices, 
                entity->meshComponent->vertexCount, 
                entity->meshComponent->indices, 
                entity->meshComponent->indexCount,
                entity->meshComponent->gpuData
                );
    
    if(entity->lightComponent->active == 1){
        setupMaterial( entity->meshComponent->gpuData,"shaders/light_vertex.glsl", "shaders/light_fragment.glsl" );
    }else if(entity->uiComponent->active == 1) {
        setupMaterial( entity->meshComponent->gpuData,"shaders/ui_vertex.glsl", "shaders/ui_fragment.glsl" );
    }else {
        setupMaterial( entity->meshComponent->gpuData,"shaders/mesh_vertex.glsl", "shaders/mesh_fragment.glsl" );
    }
}


//----------------------------------------------------------------------------------------------//
// 3D API
//----------------------------------------------------------------------------------------------//
/**
 * @brief Create a object. Used together with obj-load/parse. Expects data from obj-parser to be of type ObjData.
 * Create a object mesh
 * @param diffuse - color of the rectangle
*/
void createObject(ObjData* obj,vec3 position,vec3 scale,vec3 rotation){
    
   // vertex data
    int stride = 11;
  
    // NOT USED
    GLuint indices[] = {
        0, 1, 2,  // first triangle
        1, 2, 3   // second triangle
    }; 

    Entity* entity = addEntity(MODEL);
    
    createMesh(obj->vertexData,obj->num_of_vertices,indices,0,position,scale,rotation,&globals.materials[obj->materialIndex],GL_TRIANGLES,VERTS_COLOR_ONEUV,entity,false);
}
/**
 * @brief Create a light
 * Create a light source in the scene.
 * 
 */
void createLight(Material material,vec3 position,vec3 scale,vec3 rotation,vec3 direction,LightType type){
    
   // vertex data
    GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  1.0f,  0.0f
};
    // index data NOT USED ATM and not correct anymore
    GLuint indices[] = {
        // front and back
        0, 3, 2,
        2, 1, 0,
        4, 5, 6,
        6, 7 ,4,
        // left and right
        11, 8, 9,
        9, 10, 11,
        12, 13, 14,
        14, 15, 12,
        // bottom and top
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };
    
    // cutOff
    float degrees = 12.5f;
    float radians = DEG_TO_RAD(degrees);
    float cosine = cosf(radians);

    // outerCutOff
    float degreesOC = 17.5f;
    float radiansOC = DEG_TO_RAD(degreesOC);
    float cosineOC = cosf(radiansOC);

    Entity* entity = addEntity(MODEL);
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
    
    // TODO: This is a temporary solution, need to implement a better way to handle lights.
    globals.lights[globals.lightsCount] = *entity;
    ASSERT(globals.lightsCount < MAX_LIGHTS, "Too many lights");
    globals.lightsCount++;

    if(type == DIRECTIONAL){
        vec3 result;
        vec3 normalized_direction;
        vec3_norm(&normalized_direction,direction);
        vec3_add(&result,normalized_direction,position);
        createLine(position,result,entity);
        GLfloat positions[] = {  position[0], position[1], position[2] ,  result[0], result[1], result[2]  };
        createPoints(positions,2,entity);
        return;
    }
    createMesh(vertices,36,indices,0,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
}
/**
 * @brief Create a line segment between two points
 * @param position - start position
 * @param endPosition - end position
 */
void createLine(vec3 position, vec3 endPosition,Entity* entity){
    entity->lineComponent->active = 1;
    entity->lineComponent->start[0] = position[0];
    entity->lineComponent->start[1] = position[1];
    entity->lineComponent->start[2] = position[2];
    entity->lineComponent->end[0] = endPosition[0];
    entity->lineComponent->end[1] = endPosition[1];
    entity->lineComponent->end[2] = endPosition[2];
    entity->lineComponent->color = (Color){1.0f,1.0f,0.0f,1.0f};
    // TODO: transform pos is start of segment, should it be mid-point?
    entity->transformComponent->position[0] = position[0];
    entity->transformComponent->position[1] = position[1];
    entity->transformComponent->position[2] = position[2];
    entity->transformComponent->scale[0] = 1.0f;
    entity->transformComponent->scale[1] = 1.0f;
    entity->transformComponent->scale[2] = 1.0f;
    entity->transformComponent->rotation[0] = 0.0f;
    entity->transformComponent->rotation[1] = 0.0f;
    entity->transformComponent->rotation[2] = 0.0f;
    entity->transformComponent->modelNeedsUpdate = 1;

    GLfloat lines[] = {
        position[0], position[1], position[2], 
        endPosition[0], endPosition[1], endPosition[2]
    };

    setupLine(lines,2,entity->lineComponent->gpuData);
    setupMaterial(entity->lineComponent->gpuData,"shaders/line_vertex.glsl", "shaders/line_fragment.glsl");
}
/**
 * @brief Create a Cube
 * Create a Cube mesh
 * @param diffuse - color of the cube
*/
void createCube(Material material,vec3 position,vec3 scale,vec3 rotation){
    // vertex data
    GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  1.0f,  0.0f
};
    // index data NOT USED ATM and not correct anymore
    GLuint indices[] = {
        // front and back
        0, 3, 2,
        2, 1, 0,
        4, 5, 6,
        6, 7 ,4,
        // left and right
        11, 8, 9,
        9, 10, 11,
        12, 13, 14,
        14, 15, 12,
        // bottom and top
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };

    Entity* entity = addEntity(MODEL);
    
    createMesh(vertices,36,indices,0,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
     
}
void createPlane(Material material,vec3 position,vec3 scale,vec3 rotation){
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
    // index data NOT USED ATM
    GLuint indices[] = {
    0, 1, 2, // First triangle
    2, 3, 0  // Second triangle
    };
   
    Entity* entity = addEntity(MODEL);

    createMesh(vertices,6,indices,0,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
}
//----------------------------------------------------------------------------------------------//
// UI API
//----------------------------------------------------------------------------------------------//
int ui_createRectangle(Material material,vec3 position,vec3 scale,vec3 rotation){
    // vertex data
    GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords    // Normals
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
};
    //OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(MODEL);
    entity->uiComponent->active = 1;
    entity->uiComponent->boundingBox = (Rectangle){0.0f,0.0f,0.0f,0.0f};
    entity->uiComponent->uiNeedsUpdate = 1;
    
    createMesh(vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true);

    return entity->id;
}
/**
 * @brief Create a button
 * Create a button mesh in ui
 * @param diffuse - color of the rectangle
*/
void ui_createButton(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,ButtonCallback onClick){
    // vertex data
        GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords    // Normals
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
};
    // OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(MODEL);
    
    entity->uiComponent->active = 1;
    entity->uiComponent->boundingBox = (Rectangle){0.0f,0.0f,0.0f,0.0f};
    entity->uiComponent->text = text;
    entity->uiComponent->uiNeedsUpdate = 1;
    entity->uiComponent->onClick = onClick;
    entity->uiComponent->type = UITYPE_BUTTON;
    
    createMesh(vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 
}

/**
 * @brief Create a input field
 * Create a input mesh in ui
 * @param diffuse - color of the rectangle
*/
void ui_createTextInput(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,OnChangeCallback onChange){
    // vertex data
        GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords    // Normals
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
};
    // OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(MODEL);
    
    entity->uiComponent->active = 1;
    entity->uiComponent->boundingBox = (Rectangle){0.0f,0.0f,0.0f,0.0f};
    entity->uiComponent->text = text;
    entity->uiComponent->uiNeedsUpdate = 1;
    entity->uiComponent->onChange = onChange;
    entity->uiComponent->type = UITYPE_INPUT;

    createMesh(vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 
}