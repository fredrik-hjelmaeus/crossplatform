#include "input.h"
#include "fh-sdl.h"
#include "utils.h"

void handleKeyInput(void);
void recalculateViewports(int w, int h,struct Globals *globals);
void calcCameraFront(Camera *camera, float xpos, float ypos);

/**
 * Atm we handle all events right here. Eventually we wan't to store the polled events in a queue and process them in the update loop instead.
 * This is because this fn will become too large otherwise,and we wan't to group & handle events in a more structured way. 
 * For example, window resize events in perhaps a windowSystem, input affecting camera in a cameraSystem etc.
 */
void input(struct Globals *globals) {
    
    // Process events
    while(pollEvent(globals)) {
        
        if(globals->event.type == SDL_QUIT) {
            globals->running = 0;
        } 
        if (globals->event.type == SDL_MOUSEBUTTONDOWN) {
            
            // A button was pressed
            if (globals->event.button.button == SDL_BUTTON_LEFT) {
               
                if(globals->event.button.clicks == 2){
                   
                    globals->mouseDoubleClick = true;
                }
                
                // Left Button Pressed
                globals->mouseLeftButtonPressed = true;
                
            } else if (globals->event.button.button == SDL_BUTTON_RIGHT) {
                
              
            }
        } 
        if(globals->event.type == SDL_KEYDOWN) {
            // If input element is focused, we don't handle keyboard events here but in the uiInputSystem
            if(globals->focusedEntityId != -1){
                return;
            }else {
                handleKeyInput();
            }
        }
            
        if(globals->event.type == SDL_WINDOWEVENT && globals->event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
            int w, h; 
            getWindowSize(globals, &w, &h);
            recalculateViewports(w,h,globals);
            
            // TODO: use this for reprojection later: float aspect = (float)w / (float)h;
            //glViewport(0, 0, w / 2, h);
        }
       
        if (globals->event.type == SDL_MOUSEBUTTONUP) {
           // A button was released
           globals->mouseLeftButtonPressed = false;
           
           if(globals->cursorSelectionActive){
                globals->deselectCondition = true;
           }
           globals->mouseDragged = false;
           if(globals->focusedEntityId != -1 && globals->entities[globals->focusedEntityId].uiComponent->type == UITYPE_SLIDER){
                globals->focusedEntityId = -1;
           }
           
            
        }
        if (globals->event.type == SDL_MOUSEMOTION) {
            if(globals->mouseLeftButtonPressed){
                globals->mouseDragged = true;
            }

            int xpos = globals->event.motion.x;
            int ypos = globals->event.motion.y;

            globals->mouseXpos = (float)xpos;
            globals->mouseYpos = (float)ypos;

            // -----------TEMP CODE------------
          /*   printf("Mouse SDL coords: %d, %d\n", xpos, ypos);
            // mouse move in ndc coordinates
              float x_ndc = (2.0f * xpos) / width - 1.0f;
              float y_ndc = 1.0f - (2.0f * ypos) / height;
              float x_ui =  (-1 * x_ndc) * ((float)width * 0.5);
              float y_ui =   y_ndc * ((float)height * 0.5);  
            //  printf("Mouse moved to NDC %f, %f\n", x_ndc, y_ndc);
            //printf("Mouse moved to %f, %f\n", globals->mouseXpos, globals->mouseYpos);
            SDLVector2 sdlMouseCoords = {xpos, ypos};
            UIVector2 uiCoords = convertSDLToUI(sdlMouseCoords, width, height);
            printf("Mouse UI coords: %f, %f\n", uiCoords.x, uiCoords.y); */
            // END TEMP CODE------------------

            // Not sure this is used since we don't use viewports now?
            if(isPointInsideRect(globals->views.main.rect, (vec2){xpos, ypos})){ 
               // printf("Mouse is within main view\n ");
                // Update the camera front vector
                calcCameraFront(globals->views.main.camera,xpos, ypos);
                // set view matrix needs update flag (so that we recalculate the view matrix with the new front vector)
                globals->views.main.camera->viewMatrixNeedsUpdate = 1;
            }
            if(isPointInsideRect(globals->views.ui.rect, (vec2){xpos, ypos})){
                globals->views.ui.isMousePointerWithin = true;
            }else{
                globals->views.ui.isMousePointerWithin = false;
            }

           
        }
    }
}
