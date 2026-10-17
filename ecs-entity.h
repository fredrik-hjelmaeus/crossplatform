#ifndef ECS_ENTITY_H
#define ECS_ENTITY_H

typedef struct AppState AppState;

#include "ecs.h"

Entity* addEntity(AppState *app, enum Tag tag);
void deleteEntity(Entity* entity);

// get entity by id
// get entities by tag

#endif // ECS_ENTITY_H

