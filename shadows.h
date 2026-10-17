#ifndef SHADOWS_H
#define SHADOWS_H

typedef struct RenderState RenderState;


typedef struct AppState AppState;

void createDirectionalLightSpace(RenderState *renderState, AppState *app, int index);

#endif // SHADOWS_H
