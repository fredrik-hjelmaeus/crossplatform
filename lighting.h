#ifndef LIGHTING_H
#define LIGHTING_H

typedef struct RenderState RenderState;


typedef struct AppState AppState;

void createPointLightSpace(RenderState *renderState, AppState *app, int index);
void createSpotLightSpace(RenderState *renderState, AppState *app, int index);
void createLightSpace(RenderState *renderState, AppState *app);

#endif // LIGHTING_H
