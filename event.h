#ifndef EVENT_H
#define EVENT_H

typedef struct AppState AppState;

/**
 * How it works:
 *  EventQueue & Subscribers array is the "Announcer"
 *  pushEvent is the "Sender"
 *  EventCallback is the "Receiver"
 *  Association is the "Who is who"
 * 
 * See tests_events for example of setting up event
 * & it's lifecycle.
 */

typedef enum {
  EVENT_NONE,
  EVENT_LIGHT_DIR_X_CHANGED,
  EVENT_LIGHT_DIR_Y_CHANGED,
  EVENT_TOGGLE_UI_PANEL,
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

typedef enum {
  TRANSFORM_POS_X,
  TRANSFORM_POS_Y,
} TransformType;

typedef struct {
  int objectEntityId;
  float t;
} TransformPayload;

typedef struct {
  EventType type;
  union {
    NotificationPayload notification;
    TransformPayload transform;
    // Add all payload shapes you need here...
  } payload;
} Event;

typedef void(*EventCallback)(AppState *app, Event event, void* association);

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
void processEvents(AppState *app);
void dispatchEvent(AppState *app, Event event);

void subscribeToEvent(Subscriber sub);

/**
 * Receiver callback functions.
 */
void onDirectionLightChangeX(AppState *app, Event event, void* association);
void onDirectionLightChangeY(AppState *app, Event event, void* association);

#endif // EVENT_H

