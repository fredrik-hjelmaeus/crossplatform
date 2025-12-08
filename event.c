#include "event.h"
#include "globals.h"
#include "input-mapping.h"
#include "ui.h"

// Event queue
Event eventQueue[MAX_EVENTS_IN_QUEUE];
int eventQueueCount = 0;

// Subscribers 
Subscriber subscribers[EVENT_COUNT][MAX_SUBSCRIBERS_PER_EVENT];
int subscriberCounts[EVENT_COUNT] = {0};

// Action fn's
void onDirectionLightChange(Event event, void* association){
  ASSERT(event.type == EVENT_LIGHT_DIR_CHANGED,"wrong event type");
  ASSERT(association != NULL,"association not provided (NULL)");
  
  if(event.type == EVENT_LIGHT_DIR_CHANGED){
    
    LightDirectionAssociation* lightAssoc = (LightDirectionAssociation*)association;

    ASSERT(lightAssoc->lightEntityId != -1, "lightEntityId in association is invalid");
    ASSERT(lightAssoc->sliderEntityId != -1, "sliderEntityId in association is invalid");
    ASSERT(lightAssoc->textInputEntityId != -1, "textInputEntityId in association is invalid");

    // Update light value
    if(globals.entities[lightAssoc->lightEntityId].transformComponent->active){
      globals.entities[lightAssoc->lightEntityId].lightComponent->direction[1] = event.payload.translateXObject.x;
    }
    
    // Update ui slider value
    if(globals.entities[lightAssoc->sliderEntityId].uiComponent->active){
      globals.entities[lightAssoc->sliderEntityId].uiComponent->sliderValue = event.payload.translateXObject.x;
    }
    
    // Update ui text input value
    if(globals.entities[lightAssoc->textInputEntityId].alive && globals.entities[lightAssoc->textInputEntityId].uiComponent->active){
      snprintf(globals.entities[lightAssoc->textInputEntityId].uiComponent->text,MAX_TEXT_LENGTH,"%0.2f",event.payload.translateXObject.x);
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

void testEvents(){
   // This is a test setup aswell as doc for event-flow.
   // Meant to be used mainly for one-entity or one-time occurances.
   // This leaves systems to do updates on batches and we can avoid having to
   // shoe-horn in these one-time updates we now can use event flows for.
   
   // There are several event flows.
   // 1. ui mapping, where we setup an association struct where we provide the actors id for 
   // that particular event. 
   // The function we provide should know what to do with actors id and provided event payload.
   // 2. input mapping for keyboard input & mouse input etc.

   // Test of 1. ui mapping:
   changeSceneMode(SCENE_MODE_NORMAL); // sets mode
   
   LightDirectionAssociation* lightAss = (LightDirectionAssociation*)malloc(sizeof(LightDirectionAssociation));
   
   lightAss->lightEntityId = 2;
   lightAss->sliderEntityId = 3;
   lightAss->textInputEntityId = 4;

   Subscriber sub = { .eventType = EVENT_LIGHT_DIR_CHANGED, .callback = onDirectionLightChange, .association = lightAss };
   
   subscribeToEvent(sub); // event sub
   
   // Fake invoke phase two. When slider or textInput has found an update on itself, it will send out this event.
   // In this case it will provide new x value. 
   // onDirectionLightChangeUpdateUI will have to know how to update lightAss with the payload.
   TranslateXObjectPayload tpo = {.objectEntityId = 3, .x = 1.0 };
   Event translateObjectEvent = {.type = EVENT_LIGHT_DIR_CHANGED, .payload.translateXObject = tpo };
   pushEvent(translateObjectEvent);
   
   processEvents(); // walk eventqueue and executes events

   // Test of 2. -> input-mapping -> subscribe -> process -> event -> action
   // Incomplete/Broken atm.
   //addKeyBinding(102,translateObjectEvent,SCENE_MODE_NORMAL); // key mapping
   //processKeyInput(102,globals.currentSceneMode); // trigger event / push to eventqueue
   exit(0);
}
