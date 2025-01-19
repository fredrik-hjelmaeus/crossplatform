#include "ecs-entity.h"
#include "globals.h"

Entity* addEntity(enum Tag tag){
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 0) {
            globals.entities[i].alive = 1;
            globals.entities[i].visible = 1;
            globals.entities[i].tag = tag;
            return &globals.entities[i];
        }
    }
    // if we get here we are out of entities and need to increase the MAX_ENTITIES constant 
    // for now, we just exit the program
    printf("Out of entities\n");
    exit(1);
}

void deleteEntity(Entity* entity){
    entity->alive = 0;
    entity->tag = UNINITIALIZED;
    entity->transformComponent->active = 0;
    entity->groupComponent->active = 0;
    entity->meshComponent->active = 0;
    entity->materialComponent->active = 0;
    entity->uiComponent->active = 0;
    entity->lightComponent->active = 0;
    entity->lineComponent->active = 0;
    entity->pointComponent->active = 0;
    entity->boundingBoxComponent->active = 0;
}
