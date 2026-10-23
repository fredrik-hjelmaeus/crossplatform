#ifndef CAMERA_H
#define CAMERA_H

typedef struct InputState InputState;

typedef struct AppState AppState;

#include "types.h"    
#include "assets.h"

Camera* initCamera(void);
void calcCameraFront(Camera* camera, float xpos, float ypos);
void updateCamera(InputState *inputState, Camera* camera);

void moveCameraForward(Camera *camera, float deltaTime);
void moveCameraBackward(Camera *camera, float deltaTime);
void strafeCameraLeft(Camera *camera, float deltaTime);
void strafeCameraRight(Camera *camera, float deltaTime);
void setCameraOrthographic(Camera *camera);
void setCameraPerspective(Camera *camera);
void toggleCameraMode(Camera *camera);

#endif // CAMERA_H

