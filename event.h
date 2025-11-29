#ifndef EVENT_H
#define EVENT_H

typedef enum {
  EVENT_NONE,
  EVENT_REQUEST_LIGHT_DIR_CHANGE,
  EVENT_NOTIFY_LIGHT_DIR_CHANGED,
  EVENT_TOGGLE_PANEL,
  EVENT_TEXT_INPUT,
  EVENT_NUMBER_INPUT,
  EVENT_TOGGLE_GLOBAL_SHADOWS,
  EVENT_TOGGLE_CAST_SHADOW,
  EVENT_COUNT,
} EventType;

typedef struct {
  int triggerEntityId;
  float newValue;
} NotificationPayload;

typedef struct {
  int objectEntityId;
  float x;
} TranslateXObjectPayload;

typedef struct {
  EventType type;
  union {
    NotificationPayload notification;
    TranslateXObjectPayload translateXObject;
    // Add all payload shapes you need here...
  } payload;
} Event;

#define MAX_EVENTS_IN_QUEUE 100

extern Event eventQueue[MAX_EVENTS_IN_QUEUE];
extern int eventQueueCount;

void pushEvent(Event event);
void processEvents();

typedef void(*EventCallback)(Event event);
#define MAX_SUBSCRIBERS_PER_EVENT 8

void subscribeToEvent(EventType eventType, EventCallback callback);
void dispatchEvent(Event event);

void onDirectionalLightChange(Event event);
void onDirectionLightChangeUpdateSlider(Event event);
void onDirectionLightChangeUpdateInput(Event event);

void testEvents();

#endif // EVENT_H

