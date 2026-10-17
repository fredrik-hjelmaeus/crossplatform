#ifndef ECS_SYSTEMS_H
#define ECS_SYSTEMS_H

typedef struct RenderState RenderState;

typedef struct AppState AppState;

//#include "event-system.h"

void cameraSystem(AppState *app);
void uiPositionSystem(AppState *app);
void movementSystem(AppState *app);
void uiInputSystem(AppState *app);
void hoverAndClickSystem(AppState *app);
void textCursorSystem(AppState *app);
void modelSystem(AppState *app);
void debugSystem(RenderState *renderState);
void uiSliderSystem(AppState *app);
void uiCheckboxSystem(RenderState *renderState, AppState *app);

#endif
