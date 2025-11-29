#include "input-mapping.h"
#include "event.h"
#include <stdio.h>

// This nested structure is more performant at runtime,
// than a single flat list and having mode as prop in KeyBinding.
KeyBinding keyBindings[SCENE_MODE_COUNT][MAX_KEY_BINDINGS_PER_SCENE_MODE];
int numKeyBindings[SCENE_MODE_COUNT] = {0};

void addKeyBinding(int key, Event event, SceneMode mode){
  
  if(mode == SCENE_MODE_NONE || mode == SCENE_MODE_COUNT){
    return;
  }

  if(numKeyBindings[mode] > MAX_KEY_BINDINGS_PER_SCENE_MODE){
    printf("max key binds reached for this mode %i \n",mode);
    return;
  }
  
  keyBindings[mode][numKeyBindings[mode]].key = key;
  keyBindings[mode][numKeyBindings[mode]].event = event;
  numKeyBindings[mode]++;
}

void processKeyInput(int key,SceneMode currentSceneMode){
  for(int i = 0; i < numKeyBindings[currentSceneMode]; i++){
    if(keyBindings[currentSceneMode][i].key == key){
      pushEvent(keyBindings[currentSceneMode][i].event);
    }
  }
}

