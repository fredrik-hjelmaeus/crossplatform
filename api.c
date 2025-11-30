#include "api.h"
#include "globals.h"
#include "opengl.h"
#include "ecs-entity.h"

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
    entity->meshComponent->vertices = (Vertex*)verts; // double cast see createMesh invoke, remove this!!!
    
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
    
    if(entity->lightComponent->active == 1){
        setupMaterial( entity->meshComponent->gpuData,"shaders/light_vertex.glsl", "shaders/light_fragment.glsl" );
    }else if(entity->materialComponent->isPostProcessMaterial) {
        //setupMaterial( entity->meshComponent->gpuData,"shaders/mesh_vertex.glsl", "shaders/mesh_fragment.glsl" );
        setupMaterial( entity->meshComponent->gpuData,"shaders/ui_vertex.glsl", "shaders/ui_fragment.glsl" );
        printf("post process mesh quad material id %d \n",entity->meshComponent->gpuData->shaderProgram);
        //setupMaterial(&globals.postProcessBuffer, "shaders/mesh_vertex.glsl", "shaders/mesh_fragment.glsl");
    }else if(entity->uiComponent->active == 1){
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
  //  int stride = 11;
  
    // NOT USED
    GLuint indices[] = {
        0, 1, 2,  // first triangle
        1, 2, 3   // second triangle
    }; 

    Entity* entity = addEntity(MODEL);
    
    createMesh((GLfloat*)obj->vertexData,obj->num_of_vertices,indices,0,position,scale,rotation,&globals.materials[obj->materialIndex],GL_TRIANGLES,VERTS_COLOR_ONEUV,entity,false);
}
/**
 * @brief Create a light
 * Create a light source in the scene.
 * @return {int} Entity id
 */
int createLight(Material material,vec3 position,vec3 scale,vec3 rotation,vec3 direction,LightType type){
    
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
    entity->lightComponent->castShadows = true;
    
    // TODO: This is a temporary solution, need to implement a better way to handle lights.
    globals.lights[globals.lightsCount].entityId = entity->id;
    globals.lights[globals.lightsCount].type = type;
    ASSERT(globals.lightsCount < MAX_LIGHTS, "Too many lights");
    globals.lightsCount++;

    if(type == DIRECTIONAL){
        vec3 result;
        vec3 normalized_direction;
        vec3_norm(normalized_direction,direction);
        float scalar = 10.0f;
        vec3 scaled_direction;
        vec3_scale(scaled_direction, normalized_direction, scalar);
        // multiply normalized direction with a float here
        vec3_add(result, scaled_direction, position);
        createLine(position, result, entity);
        //GLfloat positions[] = { position[0], position[1], position[2] , result[0], result[1], result[2] };
   //     createPoints(positions,2,entity);
        setTransformData(entity,position,scale,rotation);
       //  createMesh(vertices,36,indices,0,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
       createMesh(vertices,36,indices,0,result,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
        
       // return;
    }
    //createMesh(vertices,36,indices,0,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
    return entity->id;
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

    material.diffuseMapOpacity = 1.0;
    material.material_flags |= MATERIAL_DIFFUSEMAP_ENABLED;
    
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

    material.diffuseMapOpacity = 1.0;
    material.material_flags |= MATERIAL_DIFFUSEMAP_ENABLED;
   
    Entity* entity = addEntity(MODEL);

    createMesh(vertices,6,indices,0,position,scale,rotation,&material,GL_TRIANGLES,VERTS_ONEUV,entity,true);
}

// TODO: unfinished. Supposed to draw the frustum of the camera or light correctly.
void debug_drawFrustum()
{
     //float near_plane = -0.05f, far_plane = 300.0f;
    //mat4x4_ortho(lightProjection, -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    Entity* entity = addEntity(MODEL);
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
    createLine(startt,endt,entity);
    
    // Near plane bottom
    Entity* entity4 = addEntity(MODEL);
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
    createLine(startb,endb,entity4);
   
    // Near plane right
    Entity* entity2 = addEntity(MODEL);
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
    createLine(startr,endr,entity2);

    // Near plane left
    Entity* entity3 = addEntity(MODEL);
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
    createLine(startl,endl,entity3);
    
}
