#include "event.h"
#include "globals.h"
#include "input-mapping.h"
#include "ui.h"
#include "utils.h"

// Event queue
Event eventQueue[MAX_EVENTS_IN_QUEUE];
int eventQueueCount = 0;

// Subscribers 
Subscriber subscribers[EVENT_COUNT][MAX_SUBSCRIBERS_PER_EVENT];
int subscriberCounts[EVENT_COUNT] = {0};

// Action fn's
void onDirectionLightChangeX(Event event, void* association){
  ASSERT(event.type == EVENT_LIGHT_DIR_X_CHANGED,"wrong event type");
  ASSERT(association != NULL,"association not provided (NULL)");
  
  if(event.type == EVENT_LIGHT_DIR_X_CHANGED){
    
    LightDirectionAssociation* lightAssoc = (LightDirectionAssociation*)association;

    ASSERT(lightAssoc->lightEntityId != -1, "lightEntityId in association is invalid");
    ASSERT(lightAssoc->sliderEntityId != -1, "sliderEntityId in association is invalid");
    ASSERT(lightAssoc->textInputEntityId != -1, "textInputEntityId in association is invalid");

    int entityId = event.payload.transform.objectEntityId;
    Entity* focusedEntity = &globals.entities[entityId];

    ASSERT(focusedEntity != NULL,"No focused Entity found");
    ASSERT(focusedEntity->transformComponent != NULL,"Missing transformcomponent");

    // Update light value
    if(globals.entities[lightAssoc->lightEntityId].transformComponent->active){
      globals.entities[lightAssoc->lightEntityId].transformComponent->position[0] = event.payload.transform.t * 10.0;
    }
    
    // Update ui slider value
    if(globals.entities[lightAssoc->sliderEntityId].uiComponent->active){
      // Slider position change
      float newMouseX = (float)globals.mouseXpos;
      float newPosX = absValue(newMouseX) - (width / 2);

      focusedEntity->transformComponent->position[0] = newPosX;
      focusedEntity->transformComponent->modelNeedsUpdate = 1;

      globals.entities[focusedEntity->uiComponent->boundingBoxEntityId].transformComponent->position[0] = newPosX;
      globals.entities[focusedEntity->uiComponent->boundingBoxEntityId].transformComponent->modelNeedsUpdate = 1;

      focusedEntity->boundingBoxComponent->boundingBox.min[0] = newMouseX - 10;
      focusedEntity->boundingBoxComponent->boundingBox.max[0] = newMouseX + focusedEntity->transformComponent->scale[0] -10; 
    }
    
    // Update ui text input value
    if(globals.entities[lightAssoc->textInputEntityId].alive && globals.entities[lightAssoc->textInputEntityId].uiComponent->active){
      snprintf(globals.entities[lightAssoc->textInputEntityId].uiComponent->text,MAX_TEXT_LENGTH,"%0.2f",event.payload.transform.t);
    }
  }
 
}
void onDirectionLightChangeY(Event event, void* association){
  ASSERT(event.type == EVENT_LIGHT_DIR_Y_CHANGED,"wrong event type");
  ASSERT(association != NULL,"association not provided (NULL)");
  
  if(event.type == EVENT_LIGHT_DIR_Y_CHANGED){
    
    LightDirectionAssociation* lightAssoc = (LightDirectionAssociation*)association;

    ASSERT(lightAssoc->lightEntityId != -1, "lightEntityId in association is invalid");
    ASSERT(lightAssoc->sliderEntityId != -1, "sliderEntityId in association is invalid");
    ASSERT(lightAssoc->textInputEntityId != -1, "textInputEntityId in association is invalid");

    // Update light value
    if(globals.entities[lightAssoc->lightEntityId].transformComponent->active){
      globals.entities[lightAssoc->lightEntityId].transformComponent->position[1] = event.payload.transform.t * 10.0;
    }
    
    // Update ui slider value
    if(globals.entities[lightAssoc->sliderEntityId].uiComponent->active){
      globals.entities[lightAssoc->sliderEntityId].uiComponent->sliderValue = event.payload.transform.t;
    }
    
    // Update ui text input value
    if(globals.entities[lightAssoc->textInputEntityId].alive && globals.entities[lightAssoc->textInputEntityId].uiComponent->active){
      snprintf(globals.entities[lightAssoc->textInputEntityId].uiComponent->text,MAX_TEXT_LENGTH,"%0.2f",event.payload.transform.t);
    }
  }
 
}

// Register event subscription
void subscribeToEvent(Subscriber sub) {
    int count = subscriberCounts[sub.eventType];

    ASSERT(count < MAX_SUBSCRIBERS_PER_EVENT,"max sub per event reached");

    if (count < MAX_SUBSCRIBERS_PER_EVENT) {
        subscribers[sub.eventType][count].callback = sub.callback;
        subscribers[sub.eventType][count].association = sub.association;
        subscriberCounts[sub.eventType]++;
    }
}

void dispatchEvent(Event event){
  for(int i = 0; i < subscriberCounts[event.type]; i++){
    subscribers[event.type][i].callback(event, subscribers[event.type][i].association);
  }
}

void processEvents(){
  for(int i = 0; i < eventQueueCount; i++){
    dispatchEvent(eventQueue[i]);
  }

  eventQueueCount = 0;
}

void pushEvent(Event event){

  #ifdef DEV_MODE
    ASSERT(eventQueueCount < MAX_EVENTS_IN_QUEUE, "Event queue overflow!");
  #endif
  

  if(eventQueueCount >= MAX_EVENTS_IN_QUEUE){
    return;
  }
  eventQueue[eventQueueCount] = event;
  eventQueueCount++;
}
