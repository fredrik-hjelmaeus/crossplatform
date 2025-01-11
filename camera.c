#include "camera.h"

/**
 * @brief Create a camera
 */
Camera* initCamera() {
    Camera* camera = (Camera*)malloc(sizeof(Camera));
    if (camera == NULL) {
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
void updateCamera(Camera* camera){
    if(camera->mode == CAMERAMODE_FPS){
        // Look for view needs update flag & update/recalc view matrix if needed.
        if(camera->viewMatrixNeedsUpdate == 1){
            // create view/camera transformation
            mat4x4 view;
            mat4x4_identity(view);

            // target / center 
            camera->target[0] = camera->position[0] + camera->front[0];
            camera->target[1] = camera->position[1] + camera->front[1];
            camera->target[2] = camera->position[2] + camera->front[2];

            // camera up
            mat4x4_look_at(view,camera->position, camera->target, camera->up);

            // Copy the view matrix to camera.view
            memcpy(camera->view, view, sizeof(mat4x4));

            camera->viewMatrixNeedsUpdate = 0;
        }

        // Look for projection needs update flag & update/recalc projection matrix if needed.
        // There are 4 scenarios where you would need to recalc. the projection matrix:
        // - Window Resize / Change in aspect ratio
        // - Camera projection type change, perspective to orthographic or vice versa
        // - Change in FOV
        // - Change in near/far plane
        if(camera->projectionMatrixNeedsUpdate == 1){ 
            mat4x4 projection;
            mat4x4_identity(projection);

            if (camera->isOrthographic) {
                // Calculate orthographic projection matrix
                mat4x4_ortho(projection, camera->left, camera->right, camera->bottom, camera->top, camera->near, camera->far);
            } else {
                // Calculate perspective projection matrix
                mat4x4_perspective(projection, camera->fov, camera->aspectRatio, camera->near, camera->far);
            }
            
            // Copy the projection matrix to camera.projection
            memcpy(camera->projection, projection, sizeof(mat4x4));

            camera->projectionMatrixNeedsUpdate = 0;
        }
    }
    if(camera->mode == CAMERAMODE_ORBITAL){
      
        if(globals.mouseLeftButtonPressed && globals.mouseDragged){
            if(globals.mouseDragStart[0] < -9999.0f){
                globals.mouseDragStart[0] = globals.mouseXpos;
                globals.mouseDragStart[1] = globals.mouseYpos;
      //          printf("drag start %f %f \n",globals.mouseDragStart[0],globals.mouseDragStart[1]);
            }
           
            float newMouseX = (float)globals.mouseXpos;
            float newMouseY = (float)globals.mouseYpos;
            float changeXThisFrame = newMouseX - globals.mouseDragPreviousFrame[0];
            float changeYThisFrame = newMouseY - globals.mouseDragPreviousFrame[1];

            globals.mouseDragPreviousFrame[0] = newMouseX;
            globals.mouseDragPreviousFrame[1] = newMouseY;

            float changeXThisMouseDown = (float)globals.mouseXpos - globals.mouseDragStart[0];
            float changeYThisMouseDown = (float)globals.mouseYpos - globals.mouseDragStart[1];

           // float changeThisFrameX = 0.01;
          //  float changeThisFrameY = 0.01;

          /*   printf("changeXThisMouseDown %f \n",changeXThisMouseDown);
            printf("changeYThisMouseDown %f \n",changeYThisMouseDown);
            printf("changeXThisFrame %f \n",changeXThisFrame);
            printf("changeYThisFrame %f \n",changeYThisFrame); */
         /*    if(changeXThisFrame > 0){
                printf("right \n");
                changeThisFrameX = -0.01;
            }else{
                printf("left \n");
            }
            if(changeYThisFrame > 0){
                printf("down \n");
                changeThisFrameY = -0.01;
            }else{
                printf("up \n");
            } */

          

      

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

            // create view/camera transformation
            mat4x4 view;
            mat4x4_identity(view);

            // camera up
            mat4x4_look_at(view,camera->position, camera->target, camera->up);

            // Copy the view matrix to camera.view
            memcpy(camera->view, view, sizeof(mat4x4));

            camera->viewMatrixNeedsUpdate = 0; 

           // printf("new camera position %f %f %f \n",camera->position[0],camera->position[1],camera->position[2]);

         //   printf("changeThisFrameX %f \n",changeThisFrameX);
        }else {
            if(globals.mouseDragStart[0] > -9999.0f){
                float xDragOffset = globals.mouseXpos - globals.mouseDragStart[0];
                float yDragOffset = globals.mouseYpos - globals.mouseDragStart[1];
            //    printf("we dragged %f %f\n", xDragOffset, yDragOffset);
                globals.mouseDragStart[0] = -10000.0f;
                globals.mouseDragStart[1] = -10000.0f;
                globals.mouseDragPreviousFrame[0] = 0.0f;
                globals.mouseDragPreviousFrame[1] = 0.0f;
            }
              // create view/camera transformation
          /*   mat4x4 view;
            mat4x4_identity(view);

            // camera up
            mat4x4_look_at(view,camera->position, camera->target, camera->up);

            // Copy the view matrix to camera.view
            memcpy(camera->view, view, sizeof(mat4x4));

            camera->viewMatrixNeedsUpdate = 0; */
        }
    }
}
