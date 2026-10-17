#ifndef CAMERA_H
#define CAMERA_H

typedef struct AppState AppState;

#include "types.h"    
#include "assets.h"

Camera* initCamera(void);
void updateCamera(AppState *app, Camera* camera);

#endif // CAMERA_H

