#include "input-state.h"
#include "render.h"
#include "app.h"
#include "input.h"
#include "fh-sdl.h"
#include "utils.h"
#include "viewport.h"
#include "camera.h"
#include "input-mapping.h"
#include "ecs.h"

static void handleKeyInput(InputState *inputState, RenderState *renderState, AppState *app);

/**
 * Atm we handle all events right here. Eventually we wan't to store the polled events in a queue and process them in the update loop instead.
 * This is because this fn will become too large otherwise,and we wan't to group & handle events in a more structured way. 
 * For example, window resize events in perhaps a windowSystem, input affecting camera in a cameraSystem etc.
 */
void input(InputState *inputState, RenderState *renderState, AppState *app) {
    
    // Process events
    while(pollEvent(inputState)) {
        
        if(inputState->event.type == SDL_QUIT) {
            app->running = 0;
        } 
        if (inputState->event.type == SDL_MOUSEBUTTONDOWN) {
            
            // A button was pressed
            if (inputState->event.button.button == SDL_BUTTON_LEFT) {
               
                if(inputState->event.button.clicks == 2){
                   
                    inputState->mouseDoubleClick = true;
                }
                
                // Left Button Pressed
                inputState->mouseLeftButtonPressed = true;
                
            } else if (inputState->event.button.button == SDL_BUTTON_RIGHT) {
                
              
            }
        } 
        if(inputState->event.type == SDL_KEYDOWN) {
            // If input element is focused, we don't handle keyboard events here but in the uiInputSystem
            if(app->focusedEntityId != -1){
                return;
            }else {
                handleKeyInput(inputState, renderState, app);
            }
        }
            
        if(inputState->event.type == SDL_WINDOWEVENT && inputState->event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
            int w, h; 
            getWindowSize(app, &w, &h);
            recalculateViewports(w,h,app);
            
            // TODO: use this for reprojection later: float aspect = (float)w / (float)h;
            //glViewport(0, 0, w / 2, h);
        }
       
        if (inputState->event.type == SDL_MOUSEBUTTONUP) {
           // A button was released
           inputState->mouseLeftButtonPressed = false;
           
           if(app->cursorSelectionActive){
                app->deselectCondition = true;
           }
           inputState->mouseDragged = false;
           if(app->focusedEntityId != -1 && app->entities[app->focusedEntityId].uiComponent->type == UITYPE_SLIDER){
                app->focusedEntityId = -1;
           }
           
            
        }
        if (inputState->event.type == SDL_MOUSEMOTION) {
            if(inputState->mouseLeftButtonPressed){
                inputState->mouseDragged = true;
            }

            int xpos = inputState->event.motion.x;
            int ypos = inputState->event.motion.y;

            inputState->mouseXpos = (float)xpos;
            inputState->mouseYpos = (float)ypos;

            // -----------TEMP CODE------------
          /*   printf("Mouse SDL coords: %d, %d\n", xpos, ypos);
            // mouse move in ndc coordinates
              float x_ndc = (2.0f * xpos) / width - 1.0f;
              float y_ndc = 1.0f - (2.0f * ypos) / height;
              float x_ui =  (-1 * x_ndc) * ((float)width * 0.5);
              float y_ui =   y_ndc * ((float)height * 0.5);  
            //  printf("Mouse moved to NDC %f, %f\n", x_ndc, y_ndc);
            //printf("Mouse moved to %f, %f\n", inputState->mouseXpos, inputState->mouseYpos);
            SDLVector2 sdlMouseCoords = {xpos, ypos};
            UIVector2 uiCoords = convertSDLToUI(sdlMouseCoords, width, height);
            printf("Mouse UI coords: %f, %f\n", uiCoords.x, uiCoords.y); */
            // END TEMP CODE------------------

            // Not sure this is used since we don't use viewports now?
            if(isPointInsideRect(app->views.main.rect, (vec2){xpos, ypos})){ 
               // printf("Mouse is within main view\n ");
                // Update the camera front vector
                calcCameraFront(app->views.main.camera,xpos, ypos);
                // set view matrix needs update flag (so that we recalculate the view matrix with the new front vector)
                app->views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(isPointInsideRect(app->views.ui.rect, (vec2){xpos, ypos})){
                app->views.ui.isMousePointerWithin = true;
            }else{
                app->views.ui.isMousePointerWithin = false;
            }

           
        }
    }
}

static void handleKeyInput(InputState *inputState, RenderState *renderState, AppState *app){
         
            processKeyInput(inputState->event.key.keysym.sym, app->currentSceneMode);
            
            const char *key = getKeyName(inputState->event.key.keysym.sym);
           
            if(strcmp(key, "C") == 0) {
                app->views.full.clearColor.r = randFloat(0.0,1.0);
                app->views.full.clearColor.g = randFloat(0.0,1.0);
                app->views.full.clearColor.b = randFloat(0.0,1.0);

                for(int i = 0; i < MAX_ENTITIES; i++){
                    if(app->entities[i].alive == 1 && app->entities[i].lightComponent->active == 1){
                        app->entities[i].lightComponent->ambient.r = app->views.full.clearColor.r;
                        app->entities[i].lightComponent->ambient.g = app->views.full.clearColor.g;
                        app->entities[i].lightComponent->ambient.b = app->views.full.clearColor.b;
                    }
                }
                //glClearColor(randFloat(0.0,1.0),randFloat(0.0,1.0),randFloat(0.0,1.0), 1.0);
            }
            if(strcmp(key, "T") == 0) {
                renderState->culling = !renderState->culling;
            }
            if(strcmp(key, "L") == 0) {
                // Pressing L will write all drawcalls of a frame to disk as a pngs
                renderState->debugDrawCalls = true;
                printf("Debug drawcalls enabled\n");
            }
            if(strcmp(key, "B") == 0) {
                renderState->drawBoundingBoxes = !renderState->drawBoundingBoxes;
            }
            if(strcmp(key, "Escape") == 0) {
                app->running = 0;
            }
            if(strcmp(key, "F") == 0) {
                toggleFullscreen(app);
            }
            if(strcmp(key, "W") == 0){
                app->views.main.camera->position[0] += app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[0];
                app->views.main.camera->position[1] += app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[1];
                app->views.main.camera->position[2] += app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[2];

                // set view matrix needs update flag
                app->views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "S") == 0){
                app->views.main.camera->position[0] -= app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[0];
                app->views.main.camera->position[1] -= app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[1];
                app->views.main.camera->position[2] -= app->views.main.camera->speed * app->delta_time * app->views.main.camera->front[2];

                // set view matrix needs update flag
                app->views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "A") == 0){

                // Do a cross product to get the right vector for strafing

                // allocate memory for calculation.
                float newPos[3];
                float crossProduct[3];

                // copy position into newPos
                memcpy(newPos, app->views.main.camera->position, sizeof(newPos));

                // cross product
                vec3_mul_cross(crossProduct, app->views.main.camera->front, app->views.main.camera->up);

                // normalize
                vec3_norm(newPos, crossProduct);

                // move & assign new position
                app->views.main.camera->position[0] -= app->views.main.camera->speed * app->delta_time * newPos[0];
                app->views.main.camera->position[1] -= app->views.main.camera->speed * app->delta_time * newPos[1];
                app->views.main.camera->position[2] -= app->views.main.camera->speed * app->delta_time * newPos[2];

                // set view matrix needs update flag
                app->views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "D") == 0){

                // Do a cross product to get the right vector for strafing

                // allocate memory for calculation.
                float newPos[3];
                float crossProduct[3];

                // copy position into newPos
                memcpy(newPos, app->views.main.camera->position, sizeof(newPos));

                // cross product
                vec3_mul_cross(crossProduct, app->views.main.camera->front, app->views.main.camera->up);

                // normalize
                vec3_norm(newPos, crossProduct);

                // move & assign new position
                app->views.main.camera->position[0] += app->views.main.camera->speed * app->delta_time * newPos[0];
                app->views.main.camera->position[1] += app->views.main.camera->speed * app->delta_time * newPos[1];
                app->views.main.camera->position[2] += app->views.main.camera->speed * app->delta_time * newPos[2];

                // set view matrix needs update flag
                app->views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "O") == 0){
                // Change the camera to orthographic
                app->views.main.camera->isOrthographic = 1;
                app->views.main.camera->projectionMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "P") == 0){
                // Change the camera to perspective
                app->views.main.camera->isOrthographic = 0;
                app->views.main.camera->projectionMatrixNeedsUpdate = 1;
            }
            if(strcmp(key, "I") == 0){
                if(app->views.main.camera->mode == CAMERAMODE_ORBITAL){
                    app->views.main.camera->mode = CAMERAMODE_FPS;
                }else{
                    app->views.main.camera->mode = CAMERAMODE_ORBITAL;
                    app->views.main.camera->target[0] = 0.0f;
                    app->views.main.camera->target[1] = 0.0f;
                    app->views.main.camera->target[2] = 0.0f;
                }
            }
            if(strcmp(key, "U") == 0){
                renderState->blinnMode = !renderState->blinnMode;
            }
            if(strcmp(key, "G") == 0){
                renderState->gamma = !renderState->gamma;
            }
            if(strcmp(key, "H") == 0){
                renderState->showDepthMap = !renderState->showDepthMap;
            }
            if(strcmp(key, "U") == 0){
                app->showUI = !app->showUI;
            }
}

