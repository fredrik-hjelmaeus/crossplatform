#include "event-system.h"

#define MAX_EVENTS_PER_UPDATELOOP 10 // Throttle

void eventSystem(){
/*  for(int i = 0; i < MAX_EVENTS_PER_UPDATELOOP; i++){
  executeEvent(globals.eventQueue.events[i]);
 } */
 
}

void sliderAction(Event event,int triggerEntityId){
  printf("slider action with trigger id: %d \n",triggerEntityId);
  for(int i = 0; i < event.subscriberCount; i++){
    if(event.subscribers[i] != triggerEntityId){
      // Action logic here.
      printf("action logic here on subscriber with entityId: %d \n",event.subscribers[i]);
      //globals.entities[event.subscribers[i]].tag
    }else{
      printf("no action on triggerEntityId %d \n",event.subscribers[i]);
    }
  }
}

void executeEvent(int triggerEntityId, int eventId){
   EventRegister *er = &globals.eventRegister;
   switch(er->events[eventId].type){
    case CHANGE_X_DIRECTION: sliderAction(er->events[eventId], triggerEntityId); break;
    default: printf("no event type found \n");
  }
}

/* 
void initEventQueue(){
  arena_initMemory(&globals.eventArena, EVENT_QUEUE_SIZE * sizeof(Event));
  globals.eventQueue = (EventQueue){
    .events = (Event*)arena_Alloc(&globals.eventArena,MAX_EVENTS * sizeof(Event)),
    .eventCount = 0,
    .eventCapacity = MAX_EVENTS,
  };
} */

void initEventRegister(){
  arena_initMemory(&globals.eventArena, EVENT_REGISTER_SIZE * sizeof(Event));
  globals.eventRegister = (EventRegister){
    .events = (Event*)arena_Alloc(&globals.eventArena,MAX_EVENTS * sizeof(Event)),
    .eventCount = 0,
    .eventCapacity = MAX_EVENTS,
  };
}

void triggerEvent(int triggerEntityId, int eventId){
  // Get event from EventRegister
 // EventRegister *er = &globals.eventRegister;
  // We have options here: 
  // - Execute action here directly (WHAT WE HAVE NOW IMPLEMENTED)
  executeEvent(triggerEntityId, eventId);
  //   OR we could:
  // - put on event queue to execute in steps per updateloop. (more performant?)
  // - third option is to decide how much info the event queue needs. 
  //   Will the queue do logic and prioritize certain events for exec. ?. 
  //   Then the queue needs maybe entire event struct. Otherwise it may only need
  //   the eventId, and can just lookup event from eventRegister with it instead.
  //   THIS IS NOT IMPLEMENTED
 /*  EventQueue *eq = &globals.eventQueue;
  ASSERT(eq->eventCount <= eq->eventCapacity-1, "Max events reached");
  printf("trigger event count %d \n",eq->eventCount);
  eq->events[eq->eventCount] = event;
  eq->eventCount++; */
}

int registerEvent(int eventType, int entityId){
  Event event = initEvent();
  event.type = eventType;
  event.subscribers[0] = entityId;
  event.subscriberCount++;
  // one slider change x dir on n objects
  // another slider change x dir on n other objects
  EventRegister *er = &globals.eventRegister;
  ASSERT(er->eventCount <= er->eventCapacity-1, "Max events reached");
  printf("Registered event count %d \n",er->eventCount);
  er->events[er->eventCount] = event;
  er->eventCount++;
  return event.eventId;
}

void addSubscriber(int eventId, int entityId){
  EventRegister *er = &globals.eventRegister;
  ASSERT(er->events[eventId].subscriberCount <= MAX_SUBSCRIBERS, "Max num of subscribers reached");
  printf("added subscriber %d to event %d \n",entityId,eventId);
  er->events[eventId].subscribers[er->events[eventId].subscriberCount++] = entityId;
}

int generateEventId(){
  return eventIdCount++;
}

Event initEvent() {
    Event event = {
        .eventId = generateEventId(),
        .type = EVENT_NONE,
        .subscribers = { 0 },
        .subscriberCount = 0,
    };
    return event;
}


void testEvents(){
  // Register some events
  // for(int i = 0; i < 5; i++){
   // printf("eventsys %d \n",i);
    int someEventId = registerEvent(CHANGE_X_DIRECTION,0);
    // Add subscriber(s)
    addSubscriber(someEventId,2);
    addSubscriber(someEventId,7);
 // } 

   // trigger event
    triggerEvent(2,someEventId);
}

