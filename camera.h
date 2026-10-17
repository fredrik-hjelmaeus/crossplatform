#ifndef CAMERA_H
#define CAMERA_H

typedef struct InputState InputState;

typedef struct AppState AppState;

#include "types.h"    
#include "assets.h"

Camera* initCamera(void);
void updateCamera(InputState *inputState, Camera* camera);

#endif // CAMERA_H

