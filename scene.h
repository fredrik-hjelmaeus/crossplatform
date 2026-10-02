#ifndef SCENE_H
#define SCENE_H


struct Globals;

typedef enum {
  SCENE_MODE_NONE, // No input,event or update occurs
  SCENE_MODE_NORMAL, // Input,event & update
  SCENE_MODE_LOADING, // No input or event, show loading dialog.Maybe update ?
  SCENE_MODE_COUNT,
} SceneMode;

void changeSceneMode(SceneMode mode, struct Globals *globals);
void initScene(struct Globals *globals);

#endif // SCENE_H
