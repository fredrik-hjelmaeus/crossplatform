#ifndef ECS_SYSTEMS_H
#define ECS_SYSTEMS_H

typedef struct InputState InputState;

typedef struct RenderState RenderState;

typedef struct AppState AppState;

//#include "event-system.h"

void cameraSystem(InputState *inputState, AppState *app);
void uiPositionSystem(AppState *app);
void movementSystem(AppState *app);
void uiInputSystem(InputState *inputState, AppState *app);
void hoverAndClickSystem(InputState *inputState, AppState *app);
void textCursorSystem(InputState *inputState, AppState *app);
void modelSystem(AppState *app);
void debugSystem(RenderState *renderState);
void uiSliderSystem(InputState *inputState, AppState *app);
void uiCheckboxSystem(RenderState *renderState, AppState *app);

#endif
