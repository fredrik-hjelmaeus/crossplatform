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

    ASSERT(focusedEntity != NULL, "No focused Entity found");
    ASSERT(focusedEntity->transformComponent != NULL, "Missing transformcomponent");

    // Update light value
    if(globals.entities[lightAssoc->lightEntityId].transformComponent->active){
      globals.entities[lightAssoc->lightEntityId].transformComponent->position[0] = event.payload.transform.t * 10.0;
    }
    
    // Update ui slider value
    if(globals.entities[lightAssoc->sliderEntityId].uiComponent->active){
      Entity* sliderEntity = &globals.entities[lightAssoc->sliderEntityId];
      ASSERT(sliderEntity != NULL,"missing slider entity");
      Entity* sliderBoundingBoxEntity = &globals.entities[sliderEntity->uiComponent->boundingBoxEntityId];
      ASSERT(sliderBoundingBoxEntity != NULL,"missing slider bb entity");
      
      if(entityId == lightAssoc->textInputEntityId){
        Entity* sliderRangeEntity = &globals.entities[sliderEntity->uiComponent->sliderRangeEntityId];
        ASSERT(sliderRangeEntity != NULL,"missing slider range entity");
        

        float minRange = sliderRangeEntity->boundingBoxComponent->boundingBox.min[0]; 
        float maxRange = sliderRangeEntity->boundingBoxComponent->boundingBox.max[0];
        float range = maxRange-minRange; 
        
        // Transform component use UI coordinates
        SDLVector2 sdlVec;
        sdlVec.x = minRange;
        sdlVec.y = 0.0;
        UIVector2 uiVec = convertSDLToUI(sdlVec,width,height);
        float bbWidth = sliderEntity->transformComponent->scale[0];

        // Take new t value and convert it to value of the range
        // t = 0.1 , range = 75 -> 0.1*75 = 7.5
        float xValue = (float)event.payload.transform.t * (float)range;

        // minRange + new x slider pos to get the new transform pos.
        float newXPos = (float)uiVec.x + xValue;

        sliderEntity->transformComponent->position[0] = newXPos;
        sliderEntity->transformComponent->modelNeedsUpdate = 1;

        // The bb is for collision(hover+select)
        // Set slider bb position x 
        sliderBoundingBoxEntity->transformComponent->position[0] = newXPos;
        sliderBoundingBoxEntity->transformComponent->modelNeedsUpdate = 1;

        // Set slider bb min x and max x
        // NOTE: boundingBox use SDL coordinates
        sliderEntity->boundingBoxComponent->boundingBox.min[0] = minRange + xValue - (bbWidth * 0.5); 
        sliderEntity->boundingBoxComponent->boundingBox.max[0] = minRange + xValue + (bbWidth * 0.5); 
        
      }else {
        
        // Slider position change
        float newMouseX = (float)globals.mouseXpos;
        float newPosX = absValue(newMouseX) - (width / 2.0);

        // Visual representation
        // Set slider transform position x
        sliderEntity->transformComponent->position[0] = newPosX;
        sliderEntity->transformComponent->modelNeedsUpdate = 1;

        // The bb is for collision(hover+select)
        // Set slider bb position x 
        sliderBoundingBoxEntity->transformComponent->position[0] = newPosX;
        sliderBoundingBoxEntity->transformComponent->modelNeedsUpdate = 1;

        // Set slider bb min x and max x
        sliderEntity->boundingBoxComponent->boundingBox.min[0] = newMouseX - 10;
        sliderEntity->boundingBoxComponent->boundingBox.max[0] = newMouseX + focusedEntity->transformComponent->scale[0] -10; 

      }
    }
    
    // Update ui text input value
    if(globals.entities[lightAssoc->textInputEntityId].alive && globals.entities[lightAssoc->textInputEntityId].uiComponent->active){
      floatToString(
        event.payload.transform.t, 
        sizeof(globals.entities[lightAssoc->textInputEntityId].uiComponent->text),
        globals.entities[lightAssoc->textInputEntityId].uiComponent->text
      ); 
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

void processEvents(void){
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
