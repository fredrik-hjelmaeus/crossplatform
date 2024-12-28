#include "ecs.h"
#include "globals.h"

//
// Every entity get's all components attached to it and memory is allocated for all components.
//


/**
 * @brief Initialize the ECS
 * Allocates memory pools for the components and entities.
 * This means that for every new component type we add, entity size grows.
 * This is a tradeoff between memory and performance. 
 * Having the components already allocated on startup is faster, but uses more memory.
 * If component size grows too large, we might consider allocating them on the fly or have a memory pool for each component type?
*/
void initECS(){
    // Allocate memory for MAX_ENTITIES Components structs
    TransformComponent* transformComponents = allocateComponentMemory(sizeof(TransformComponent), "transform");
    GroupComponent* groupComponents = allocateComponentMemory(sizeof(GroupComponent), "group");
    MeshComponent* meshComponents = allocateComponentMemory(sizeof(MeshComponent), "mesh");
    MaterialComponent* materialComponents = allocateComponentMemory(sizeof(MaterialComponent), "material");
    UIComponent* uiComponents = allocateComponentMemory(sizeof(UIComponent), "ui");
    LightComponent* lightComponents = allocateComponentMemory(sizeof(LightComponent), "light");
    LineComponent* lineComponents = allocateComponentMemory(sizeof(LineComponent), "line");
    PointComponent* pointComponents = allocateComponentMemory(sizeof(PointComponent), "point");

    // Allocate memory for MAX_ENTITIES Entity structs
    Entity* entities = (Entity*)calloc(MAX_ENTITIES, sizeof(Entity));
    if(entities == NULL) {
        printf("Failed to allocate memory for entities\n");
        exit(1);
    }

    // Initialize entities
    for (int i = 0; i < MAX_ENTITIES; i++) {
        entities[i].alive = 0;
        entities[i].id = i;
        entities[i].tag = UNINITIALIZED;
        entities[i].transformComponent = &transformComponents[i];
        initializeTransformComponent(entities[i].transformComponent);
        entities[i].groupComponent = &groupComponents[i];
        initializeGroupComponent(entities[i].groupComponent);
        entities[i].meshComponent = &meshComponents[i];
        initializeMeshComponent(entities[i].meshComponent);
        entities[i].materialComponent = &materialComponents[i];
        initializeMaterialComponent(entities[i].materialComponent); 
        entities[i].uiComponent = &uiComponents[i];
        initializeUIComponent(entities[i].uiComponent);
        entities[i].lightComponent = &lightComponents[i];
        initializeLightComponent(entities[i].lightComponent);
        entities[i].lineComponent = &lineComponents[i];
        initializeLineComponent(entities[i].lineComponent);
        entities[i].pointComponent = &pointComponents[i];
        initializePointComponent(entities[i].pointComponent);
    }

    globals.entities = entities;

    if(0){
        printf("lightComponent %zu\n",sizeof(LightComponent));
        printf("transformComponent %zu\n",sizeof(TransformComponent));
        printf("meshComponent %zu\n",sizeof(MeshComponent));
        printf("materialComponent %zu\n",sizeof(MaterialComponent));
        printf("groupComponent %zu\n",sizeof(GroupComponent));
        printf("uiComponent %zu\n",sizeof(UIComponent));
        printf("lineComponent %zu\n",sizeof(LineComponent));
        printf("pointComponent %zu\n",sizeof(PointComponent));
    }
}

void* allocateComponentMemory(size_t componentSize, const char* componentName) {
    void* components = calloc(MAX_ENTITIES, componentSize);
    if(components == NULL) {
        printf("Failed to allocate memory for %s components\n", componentName);
        exit(1);
    }
    return components;
}

void initializeTransformComponent(TransformComponent* transformComponent){
    transformComponent->active = 0;
    transformComponent->position[0] = 0.0f;
    transformComponent->position[1] = 0.0f;
    transformComponent->position[2] = 0.0f;
    transformComponent->scale[0] = 1.0f;
    transformComponent->scale[1] = 1.0f;
    transformComponent->scale[2] = 1.0f;
    transformComponent->rotation[0] = 0.0f;
    transformComponent->rotation[1] = 0.0f;
    transformComponent->rotation[2] = 0.0f;
    transformComponent->modelNeedsUpdate = 1;
}

void initializeMatrix4(float (*matrix)[4][4]) {
    mat4x4_identity(*matrix);
}

void initializeGroupComponent(GroupComponent* groupComponent){
    groupComponent->active = 0;
    initializeMatrix4(&groupComponent->transform);
}

void initializeMeshComponent(MeshComponent* meshComponent){
    meshComponent->active = 0;
    meshComponent->vertices = NULL;
    meshComponent->vertexCount = 5;
    meshComponent->indices = NULL;
    meshComponent->indexCount = 0;
    meshComponent->gpuData = (GpuData*)malloc(sizeof(GpuData)); // TODO: replace with arena alloc?
    if(meshComponent->gpuData == NULL) {
        printf("Failed to allocate memory for gpuData\n");
        exit(1);
    }
    // Initialize GpuData
    meshComponent->gpuData->VAO = 0;
    meshComponent->gpuData->VBO = 0;
    meshComponent->gpuData->EBO = 0;
    meshComponent->gpuData->drawMode = GL_TRIANGLES; // Default draw mode
    meshComponent->gpuData->numIndicies = 0;
}

void initializeMaterialComponent(MaterialComponent* materialComponent){
    materialComponent->active = 0;
    materialComponent->ambient.r = 0.0f;
    materialComponent->ambient.g = 0.0f;
    materialComponent->ambient.b = 0.0f;
    materialComponent->ambient.a = 1.0f;
    materialComponent->diffuse.r = 0.0f;
    materialComponent->diffuse.g = 0.0f;
    materialComponent->diffuse.b = 0.0f;
    materialComponent->diffuse.a = 1.0f;
    materialComponent->specular.r = 0.0f;
    materialComponent->specular.g = 0.0f;
    materialComponent->specular.b = 0.0f;
    materialComponent->specular.a = 1.0f;
    materialComponent->shininess = 0.0f;
    materialComponent->diffuseMapOpacity = 0.0f;
    materialComponent->diffuseMap = 0;
    materialComponent->specularMap = 0;
    materialComponent->materialIndex = -1;
    materialComponent->material_flags |= MATERIAL_DIFFUSEMAP_ENABLED;
    materialComponent->material_flags |= MATERIAL_SPECULARMAP_ENABLED;
    materialComponent->material_flags |= MATERIAL_AMBIENTMAP_ENABLED;
    materialComponent->material_flags |= MATERIAL_SHININESSMAP_ENABLED;
    materialComponent->isPostProcessMaterial = false; //TODO: move to material_flags?
}

void initializeUIComponent(UIComponent* uiComponent){
    uiComponent->active = 0;
    uiComponent->hovered = 0;
    uiComponent->clicked = 0;
    uiComponent->boundingBox.x = 0;
    uiComponent->boundingBox.y = 0;
    uiComponent->boundingBox.width = 0;
    uiComponent->boundingBox.height = 0;
    uiComponent->text = (char*)malloc(MAX_TEXT_LENGTH * sizeof(char));
    if (uiComponent->text != NULL) {
        // Initialize the allocated memory to an empty string
        uiComponent->text[0] = '\0';
    }
    uiComponent->uiNeedsUpdate = 0;
    uiComponent->boundingBoxEntityId = -1;
    uiComponent->onClick = NULL;
    uiComponent->onChange = NULL;
    uiComponent->type = UITYPE_NONE;
}

void initializeLightComponent(LightComponent* lightComponent){
    lightComponent->active = 0;
    lightComponent->direction[0] = 0.0f;
    lightComponent->direction[1] = 0.0f;
    lightComponent->direction[2] = 0.0f;
    lightComponent->intensity = 0.0f;
    lightComponent->ambient.r = 0.0f;
    lightComponent->ambient.g = 0.0f;
    lightComponent->ambient.b = 0.0f;
    lightComponent->ambient.a = 1.0f;
    lightComponent->diffuse.r = 0.0f;
    lightComponent->diffuse.g = 0.0f;
    lightComponent->diffuse.b = 0.0f;
    lightComponent->diffuse.a = 1.0f;
    lightComponent->specular.r = 0.0f;
    lightComponent->specular.g = 0.0f;
    lightComponent->specular.b = 0.0f;
    lightComponent->specular.a = 1.0f;
}

void initializeLineComponent(LineComponent* lineComponent){
    lineComponent->active = 0;
    lineComponent->gpuData = (GpuData*)malloc(sizeof(GpuData)); // TODO: replace with arena alloc?
    if(lineComponent->gpuData == NULL) {
        printf("Failed to allocate memory for gpuData\n");
        exit(1);
    }
    // Initialize GpuData
    lineComponent->gpuData->VAO = 0;
    lineComponent->gpuData->VBO = 0;
    lineComponent->gpuData->EBO = 0;
    lineComponent->gpuData->drawMode = GL_LINES; // Default draw mode
    lineComponent->gpuData->numIndicies = 0;
    lineComponent->color.r = 0.0f;
    lineComponent->color.g = 0.0f;
    lineComponent->color.b = 0.0f;
    lineComponent->color.a = 1.0f;
    lineComponent->start[0] = 0.0f;
    lineComponent->start[1] = 0.0f;
    lineComponent->start[2] = 0.0f;
    lineComponent->end[0] = 0.0f;
    lineComponent->end[1] = 0.0f;
    lineComponent->end[2] = 0.0f;
}

void initializePointComponent(PointComponent* pointComponent){
    pointComponent->active = 0;
    pointComponent->gpuData = (GpuData*)malloc(sizeof(GpuData)); // TODO: replace with arena alloc?
    if(pointComponent->gpuData == NULL) {
        printf("Failed to allocate memory for gpuData\n");
        exit(1);
    }
    pointComponent->points = NULL;
    pointComponent->color.r = 0.0f;
    pointComponent->color.g = 0.0f;
    pointComponent->color.b = 0.0f;
    pointComponent->color.a = 1.0f;
    pointComponent->pointSize = 1.0f;
}
