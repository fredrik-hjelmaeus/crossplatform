#ifndef SCENE_H
#define SCENE_H

typedef struct RenderState RenderState;

typedef struct AppState AppState;

typedef enum {
  SCENE_MODE_NONE, // No input,event or update occurs
  SCENE_MODE_NORMAL, // Input,event & update
  SCENE_MODE_LOADING, // No input or event, show loading dialog.Maybe update ?
  SCENE_MODE_COUNT,
} SceneMode;

void changeSceneMode(SceneMode mode,AppState *app);
void initScene(RenderState *renderState, AppState *app);

#endif // SCENE_H
