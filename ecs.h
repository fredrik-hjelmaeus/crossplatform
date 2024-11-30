#ifndef ECS_H
#define ECS_H

#include <stddef.h>
#include "types.h"

// Max entities constant
#define MAX_ENTITIES 1000

// Max text length for UI components
#define MAX_TEXT_LENGTH 100

/**
 * @brief Initialize the ECS
 * Allocates memory pools for the components and entities.
 * This means that for every new component type we add, entity size grows.
 * This is a tradeoff between memory and performance. 
 * Having the components already allocated on startup is faster, but uses more memory.
 * If component size grows too large, we might consider allocating them on the fly or have a memory pool for each component type?
*/
void initECS();

void* allocateComponentMemory(size_t componentSize, const char* componentName);

void initializeTransformComponent(TransformComponent* transformComponent);
void initializeGroupComponent(GroupComponent* groupComponent);
void initializeMeshComponent(MeshComponent* meshComponent);
void initializeMaterialComponent(MaterialComponent* materialComponent);
void initializeUIComponent(UIComponent* uiComponent);
void initializeLightComponent(LightComponent* lightComponent);
void initializeLineComponent(LineComponent* lineComponent);
void initializePointComponent(PointComponent* pointComponent);

void initializeMatrix4(float (*matrix)[4][4]);

#endif