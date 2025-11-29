#ifndef INPUT_MAPPING_H
#define INPUT_MAPPING_H

#include "event.h"
#include "scene.h"

typedef struct {
  Event event;
  int key;
} KeyBinding;

#define MAX_KEY_BINDINGS_PER_SCENE_MODE 20

// Module state
extern KeyBinding keyBindings[SCENE_MODE_COUNT][MAX_KEY_BINDINGS_PER_SCENE_MODE];
extern int numKeyBindings[SCENE_MODE_COUNT];

// Module functions
void addKeyBinding(int key, Event event, SceneMode mode);
void processKeyInput(int key, SceneMode currentSceneMode);

#endif // INPUT_MAPPING_H
