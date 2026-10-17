#ifndef LIGHTING_H
#define LIGHTING_H

typedef struct AppState AppState;

void createPointLightSpace(AppState *app, int index);
void createSpotLightSpace(AppState *app, int index);
void createLightSpace(AppState *app);

#endif // LIGHTING_H
