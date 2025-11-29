#include "event.h"
#include "globals.h"
#include "input-mapping.h"

// Event queue
Event eventQueue[MAX_EVENTS_IN_QUEUE];
int eventQueueCount = 0;

// Event 
EventCallback events[EVENT_COUNT][MAX_SUBSCRIBERS_PER_EVENT];
int eventSubscriberCounts[EVENT_COUNT] = {0};

// Action fn's
void onDirectionalLightChange(Event event) {
  if(event.type == EVENT_REQUEST_LIGHT_DIR_CHANGE){
    // Action
    printf("TODO: change [direction light] here... \n");

    // Notification
    Event notificationEvent;
    notificationEvent.type = EVENT_NOTIFY_LIGHT_DIR_CHANGED;
    notificationEvent.payload.notification.newValue = event.payload.translateXObject.x;
    pushEvent(notificationEvent);
  }
}
void onDirectionLightChangeUpdateSlider(Event event){
  if(event.type == EVENT_NOTIFY_LIGHT_DIR_CHANGED){
    printf("update [SLIDER] with new value: %f \n",event.payload.notification.newValue);
  }
}
void onDirectionLightChangeUpdateInput(Event event){
  if(event.type == EVENT_NOTIFY_LIGHT_DIR_CHANGED){
    printf("update [INPUT] with new value: %f \n",event.payload.notification.newValue);
  }
}

// Register event subscription
void subscribeToEvent(EventType eventType, EventCallback callback) {
    int count = eventSubscriberCounts[eventType];
    if (count < MAX_SUBSCRIBERS_PER_EVENT) {
        events[eventType][count] = callback;
        eventSubscriberCounts[eventType]++;
    }
}

void dispatchEvent(Event event){
  for(int i = 0; i < eventSubscriberCounts[event.type]; i++){
    events[event.type][i](event);
  }
}

void processEvents(){
  for(int i = 0; i < eventQueueCount; i++){
    printf("handling event: %i \n",i);
    dispatchEvent(eventQueue[i]);
  }

  eventQueueCount = 0;
}

void pushEvent(Event event){
  printf("ok \n");

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
   // shoe-horn in these one-time updates we now can use event flow for.
   // Test mode -> input-mapping -> subscribe -> process -> event -> action
   changeSceneMode(SCENE_MODE_NORMAL); // sets mode
  
   //addKeyBinding(102,translateObjectEvent,SCENE_MODE_NORMAL); // key mapping
   
   subscribeToEvent(EVENT_REQUEST_LIGHT_DIR_CHANGE, onDirectionalLightChange); // event sub
   subscribeToEvent(EVENT_NOTIFY_LIGHT_DIR_CHANGED, onDirectionLightChangeUpdateInput); // event sub
   subscribeToEvent(EVENT_NOTIFY_LIGHT_DIR_CHANGED, onDirectionLightChangeUpdateSlider); // event sub

   TranslateXObjectPayload tpo = {.objectEntityId = 2, .x = 1.0 };
   Event translateObjectEvent = {.type = EVENT_REQUEST_LIGHT_DIR_CHANGE, .payload.translateXObject = tpo };
   pushEvent(translateObjectEvent);
   
   
   //processKeyInput(102,globals.currentSceneMode); // trigger event / push to eventqueue
   processEvents(); // walk eventqueue and execute events (onObjectTransform here)
   exit(0);
}
