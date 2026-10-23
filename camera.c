#include "input-state.h"
#include "app.h"
#include "camera.h"
#include "utils.h"

/**
 * @brief Create a camera
 */
Camera* initCamera(void) 
{
    Camera* camera = (Camera*)malloc(sizeof(Camera));
    if (camera == NULL) 
    {
        // Handle memory allocation failure
        return NULL;
    }
    vec3 position = {0.0f, 0.0f, 3.0f};
    vec3 front = {0.0f, 0.0f, -1.0f};
    vec3 up = {0.0f, 1.0f, 0.0f};
    vec3 target = {0.0f, 0.0f, 0.0f};

    // Initialize the Camera fields
    memcpy(camera->position, position, sizeof(vec3));
    memcpy(camera->front, front, sizeof(vec3));
    memcpy(camera->up, up, sizeof(vec3));
    memcpy(camera->target, target, sizeof(vec3));
    // Initialize other fields as needed
    camera->speed = 0.05f;
    camera->viewMatrixNeedsUpdate = 1;
    camera->projectionMatrixNeedsUpdate = 1;
    camera->fov = 45.0f;
    camera->near = 0.1f;
    camera->far = 1000.0f;
    camera->aspectRatio = (float)width / (float)height;
    camera->left = -1.0f;
    camera->right = 1.0f;
    camera->bottom = -1.0f;
    camera->top = 1.0f;
    camera->isOrthographic = 0;
    camera->mode = CAMERAMODE_FPS;

    return camera;
}


// There are 4 scenarios where you would need to recalc. the projection matrix:
// - Window Resize / Change in aspect ratio
// - Camera projection type change, perspective to orthographic or vice versa
// - Change in FOV
// - Change in near/far plane
void updateProjection(Camera* camera)
{
    mat4x4 projection;
    mat4x4_identity(projection);

    if (camera->isOrthographic) 
    {
        // Calculate orthographic projection matrix
        mat4x4_ortho(projection, camera->left, camera->right, camera->bottom, camera->top, camera->near, camera->far);
    } 
    else 
    {
        // Calculate perspective projection matrix
        mat4x4_perspective(projection, camera->fov, camera->aspectRatio, camera->near, camera->far);
    }
    
    // Copy the projection matrix to camera.projection
    memcpy(camera->projection, projection, sizeof(mat4x4));

    camera->projectionMatrixNeedsUpdate = 0;
}

void updateView(Camera* camera)
{
     // create view/camera transformation
    mat4x4 view;
    mat4x4_identity(view);

    // camera up
    mat4x4_look_at(view,camera->position, camera->target, camera->up);

    // Copy the view matrix to camera.view
    memcpy(camera->view, view, sizeof(mat4x4));

    camera->viewMatrixNeedsUpdate = 0;
}

void updateCamera(InputState *inputState, Camera* camera){

    if(camera->mode == CAMERAMODE_FPS)
    {
        // target / center 
        camera->target[0] = camera->position[0] + camera->front[0];
        camera->target[1] = camera->position[1] + camera->front[1];
        camera->target[2] = camera->position[2] + camera->front[2];

        // Look for view needs update flag & update/recalc view matrix if needed.
        if(camera->viewMatrixNeedsUpdate == 1)
        {
           updateView(camera);
        }

        // Look for projection needs update flag & 
        // update/recalc projection matrix if needed.
        if(camera->projectionMatrixNeedsUpdate == 1)
        { 
           updateProjection(camera);
        }
    }
    
    if(camera->mode == CAMERAMODE_ORBITAL)
    {

        // Look for projection needs update flag & 
        // update/recalc projection matrix if needed.
        if(camera->projectionMatrixNeedsUpdate == 1)
        { 
            updateProjection(camera);
        }

        // Are we dragging or not?
        if(inputState->mouseLeftButtonPressed && inputState->mouseDragged)
        {
            
            float *dragStart = inputState->mouseDragStart;
            float *prevDrag  = inputState->mouseDragPreviousFrame;
            float newMouseX =  inputState->mouseXpos;
            float newMouseY =  inputState->mouseYpos;
            
            if(dragStart[0] < -9999.0f)
            {
                dragStart[0] = inputState->mouseXpos;
                dragStart[1] = inputState->mouseYpos;
                
                prevDrag[0] = newMouseX;
                prevDrag[1] = newMouseY;
            }

            float changeXThisFrame = newMouseX - inputState->mouseDragPreviousFrame[0];
            float changeYThisFrame = newMouseY - inputState->mouseDragPreviousFrame[1];

            inputState->mouseDragPreviousFrame[0] = newMouseX;
            inputState->mouseDragPreviousFrame[1] = newMouseY;

            vec3 cameraVector;
            vec3_subtract(camera->position, camera->target, &cameraVector);
            //float radius = vec3_length(cameraVector);
            float mag = magnitude(cameraVector);
            float dir = direction(cameraVector);
            float elev = elevation(cameraVector);
           
            float changeX = M_PI / 360 * changeXThisFrame;
            float changeY = M_PI / 360 * changeYThisFrame;
            vec3 newCameraPos;
            toCartesianXYZ(mag,dir+changeX,elev+changeY,&newCameraPos);
            
            camera->position[0] = newCameraPos[0];
            camera->position[1] = newCameraPos[1];
            camera->position[2] = newCameraPos[2];

            camera->viewMatrixNeedsUpdate = 1;
        }
        else 
        {
            if(inputState->mouseDragStart[0] > -9999.0f)
            {
                inputState->mouseDragStart[0] = -10000.0f;
                inputState->mouseDragStart[1] = -10000.0f;
                inputState->mouseDragPreviousFrame[0] = -10000.0f;
                inputState->mouseDragPreviousFrame[1] = -10000.0f;
            }
        }

        if(camera->viewMatrixNeedsUpdate == 1)
        {
            updateView(camera);
        }
    }
}

/*
* @brief Calculate the front vector of the camera & assign it to the global camera struct
*/
void calcCameraFront(Camera* camera, float xpos, float ypos){
            float yaw = -90.0f;
            float pitch = 0.0f;
            float lastX = 400, lastY = 300;
            float xoffset = xpos - lastX;
            float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top

            const float sensitivity = 0.1f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            yaw   += xoffset;
            pitch += yoffset; 

            if(pitch > 89.0f)
                pitch =  89.0f;
            if(pitch < -89.0f)
                pitch = -89.0f;

            vec3 direction;
            direction[0] = cos(deg2rad(yaw)) * cos(deg2rad(pitch));
            direction[1] = sin(deg2rad(pitch));
            direction[2] = sin(deg2rad(yaw)) * cos(deg2rad(pitch));
            vec3_norm(camera->front, direction);
}


void moveCameraForward(Camera *camera, float deltaTime){
    camera->position[0] += camera->speed * deltaTime * camera->front[0];
    camera->position[1] += camera->speed * deltaTime * camera->front[1];
    camera->position[2] += camera->speed * deltaTime * camera->front[2];

    // set view matrix needs update flag
    camera->viewMatrixNeedsUpdate = 1;
}

void moveCameraBackward(Camera *camera, float deltaTime){
    camera->position[0] -= camera->speed * deltaTime * camera->front[0];
    camera->position[1] -= camera->speed * deltaTime * camera->front[1];
    camera->position[2] -= camera->speed * deltaTime * camera->front[2];

    // set view matrix needs update flag
    camera->viewMatrixNeedsUpdate = 1;
}

void strafeCameraLeft(Camera *camera, float deltaTime){
    // Do a cross product to get the right vector for strafing

    // allocate memory for calculation.
    float newPos[3];
    float crossProduct[3];

    // copy position into newPos
    memcpy(newPos, camera->position, sizeof(newPos));

    // cross product
    vec3_mul_cross(crossProduct, camera->front, camera->up);

    // normalize
    vec3_norm(newPos, crossProduct);

    // move & assign new position
    camera->position[0] -= camera->speed * deltaTime * newPos[0];
    camera->position[1] -= camera->speed * deltaTime * newPos[1];
    camera->position[2] -= camera->speed * deltaTime * newPos[2];

    // set view matrix needs update flag
    camera->viewMatrixNeedsUpdate = 1;
}

void strafeCameraRight(Camera *camera, float deltaTime){
    // Do a cross product to get the right vector for strafing

    // allocate memory for calculation.
    float newPos[3];
    float crossProduct[3];

    // copy position into newPos
    memcpy(newPos, camera->position, sizeof(newPos));

    // cross product
    vec3_mul_cross(crossProduct, camera->front, camera->up);

    // normalize
    vec3_norm(newPos, crossProduct);

    // move & assign new position
    camera->position[0] += camera->speed * deltaTime * newPos[0];
    camera->position[1] += camera->speed * deltaTime * newPos[1];
    camera->position[2] += camera->speed * deltaTime * newPos[2];

    // set view matrix needs update flag
    camera->viewMatrixNeedsUpdate = 1;
}

void setCameraOrthographic(Camera *camera){
    // Change the camera to orthographic
    camera->isOrthographic = 1;
    camera->projectionMatrixNeedsUpdate = 1;
}

void setCameraPerspective(Camera *camera){
    // Change the camera to perspective
    camera->isOrthographic = 0;
    camera->projectionMatrixNeedsUpdate = 1;
}

void toggleCameraMode(Camera *camera){
    if(camera->mode == CAMERAMODE_ORBITAL){
        camera->mode = CAMERAMODE_FPS;
    }else{
        camera->mode = CAMERAMODE_ORBITAL;
        camera->target[0] = 0.0f;
        camera->target[1] = 0.0f;
        camera->target[2] = 0.0f;
    }
    
    camera->viewMatrixNeedsUpdate = 1;
}
