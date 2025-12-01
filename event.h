#ifndef EVENT_H
#define EVENT_H

/**
 * How it works:
 *  EventQueue & Subscribers array is the "Announcer"
 *  pushEvent is the "Sender"
 *  EventCallback is the "Receiver"
 *  Association is the "Who is who"
 * 
 * See testEvents for example of setting up event
 * & it's lifecycle.
 */

typedef enum {
  EVENT_NONE,
  EVENT_LIGHT_DIR_CHANGED,
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

typedef void(*EventCallback)(Event event, void* association);

typedef struct Subscriber {
  EventType eventType;
  /**
   * EventCallback is function pointer.
   */
  EventCallback callback;
  /**
   * Association is a generic you check & cast to the type the event needs.
   */
  void* association; 
} Subscriber;

#define MAX_EVENTS_IN_QUEUE 100
#define MAX_SUBSCRIBERS_PER_EVENT 8

void pushEvent(Event event);
void processEvents();
void dispatchEvent(Event event);

void subscribeToEvent(Subscriber sub);

/**
 * Receiver callback functions.
 */
void onDirectionLightChange(Event event, void* association);

void testEvents();

#endif // EVENT_H

