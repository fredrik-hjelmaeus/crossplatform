#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include "utils.h"
#include "globals.h"
/* 
The flow: 
- Setup/Init:
- We create event, it returns eventId.
- We create slider entity, provide eventId.
- Entity uses eventId to invoke addSubscriber(eventId)

- Update/loop:
- SDL event triggers
- slider entity as triggerEntityId. 

Then we have a gameloop that checks user input, ex. mousedown. 
If mousedown is on slider entity we add its event to the event queue. 
Next eventSystem goes through the event queue and invoke the callback attached/defined in the event, 
which is a function pointer on the event. We will call those functions actions.
The function looks on the triggerId and knows what value to lookup. 
The last assignment step would then look like this: 
for each subscriber do:
globals.entities[subscriber[0]].someproperty = globals.entities[triggerEntityId].someValue 
globals.entities[subscriber[1]].someproperty = globals.entities[triggerEntityId].someValue

This pub-sub event setup solves the problem with ecs-systems, where updating a single entity or just a few entities requires us to iterate
over all entities and check if it needs to be updated. This is good when most or all entities needs update, but not otherwise.
Together with triggerId and subscriberId and globals.entities structure we can access and update in constant time on single entities.
There will have to be a decision/trade-off between when to do an event and when to do a normal ecs-system on all entities by tag or component.
For now this trade-off is on 10 subscribers or more, hence why we set MAX_SUBSCRIBERS to 10.
*/

#define MAX_EVENTS 10

static int eventIdCount = 0;
int generateEventId();
Event initEvent();
/**
 * Logic to execute on slider change.
 */
void sliderAction(Event event,int triggerEntityId);
/**
 * Runs every frame and tries to handle all events in queue and pop them off.
 */
void eventSystem();
void initEventQueue();
void initEventRegister();
/**
 * Adds event to EventRegister
 * Also adds entity as first subscriber.
 */
int registerEvent(int eventType, int entityId);
/**
 * Adds event to EventQueue
 */
void triggerEvent(int triggerEntityId, int eventId);
/**
 * Adds subscriber to provided eventId.
 */
void addSubscriber(int eventId, int entityId);
void testEvents();

#endif
