#include "render.h"
#include "app.h"
#include "input.h"
#include "fh-sdl.h"
#include "utils.h"
#include "viewport.h"

void handleKeyInput(RenderState *renderState, AppState *app);
void calcCameraFront(Camera *camera, float xpos, float ypos);

/**
 * Atm we handle all events right here. Eventually we wan't to store the polled events in a queue and process them in the update loop instead.
 * This is because this fn will become too large otherwise,and we wan't to group & handle events in a more structured way. 
 * For example, window resize events in perhaps a windowSystem, input affecting camera in a cameraSystem etc.
 */
void input(RenderState *renderState, AppState *app) {
    
    // Process events
    while(pollEvent(app)) {
        
        if(app->event.type == SDL_QUIT) {
            app->running = 0;
        } 
        if (app->event.type == SDL_MOUSEBUTTONDOWN) {
            
            // A button was pressed
            if (app->event.button.button == SDL_BUTTON_LEFT) {
               
                if(app->event.button.clicks == 2){
                   
                    app->mouseDoubleClick = true;
                }
                
                // Left Button Pressed
                app->mouseLeftButtonPressed = true;
                
            } else if (app->event.button.button == SDL_BUTTON_RIGHT) {
                
              
            }
        } 
        if(app->event.type == SDL_KEYDOWN) {
            // If input element is focused, we don't handle keyboard events here but in the uiInputSystem
            if(app->focusedEntityId != -1){
                return;
            }else {
                handleKeyInput(renderState, app);
            }
        }
            
        if(app->event.type == SDL_WINDOWEVENT && app->event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
            int w, h; 
            getWindowSize(app, &w, &h);
            recalculateViewports(w,h,app);
            
            // TODO: use this for reprojection later: float aspect = (float)w / (float)h;
            //glViewport(0, 0, w / 2, h);
        }
       
        if (app->event.type == SDL_MOUSEBUTTONUP) {
           // A button was released
           app->mouseLeftButtonPressed = false;
           
           if(app->cursorSelectionActive){
                app->deselectCondition = true;
           }
           app->mouseDragged = false;
           if(app->focusedEntityId != -1 && app->entities[app->focusedEntityId].uiComponent->type == UITYPE_SLIDER){
                app->focusedEntityId = -1;
           }
           
            
        }
        if (app->event.type == SDL_MOUSEMOTION) {
            if(app->mouseLeftButtonPressed){
                app->mouseDragged = true;
            }

            int xpos = app->event.motion.x;
            int ypos = app->event.motion.y;

            app->mouseXpos = (float)xpos;
            app->mouseYpos = (float)ypos;

            // -----------TEMP CODE------------
          /*   printf("Mouse SDL coords: %d, %d\n", xpos, ypos);
            // mouse move in ndc coordinates
              float x_ndc = (2.0f * xpos) / width - 1.0f;
              float y_ndc = 1.0f - (2.0f * ypos) / height;
              float x_ui =  (-1 * x_ndc) * ((float)width * 0.5);
              float y_ui =   y_ndc * ((float)height * 0.5);  
            //  printf("Mouse moved to NDC %f, %f\n", x_ndc, y_ndc);
            //printf("Mouse moved to %f, %f\n", app->mouseXpos, app->mouseYpos);
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
