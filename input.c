#include "input-state.h"
#include "render.h"
#include "app.h"
#include "input.h"
#include "fh-sdl.h"
#include "utils.h"
#include "viewport.h"
#include "camera.h"
#include "scene.h"
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

        switch(inputState->event.type) {
            case SDL_QUIT: app->running = 0; break;
            case SDL_MOUSEBUTTONDOWN: 
                // A button was pressed
                if (inputState->event.button.button == SDL_BUTTON_LEFT) {
                
                    if(inputState->event.button.clicks == 2)
                    {
                        inputState->mouseDoubleClick = true;
                    }
                    
                    // Left Button Pressed
                    inputState->mouseLeftButtonPressed = true;
                    
                } else if (inputState->event.button.button == SDL_BUTTON_RIGHT) {
                    
                }
            break;
            case SDL_KEYDOWN:
                // If input element is focused, 
                // we don't handle keyboard events here but 
                // in the uiInputSystem
                if(app->focusedEntityId != -1){
                    return;
                }else {
                    handleKeyInput(inputState, renderState, app);
                }
            break;
            case SDL_WINDOWEVENT:
                if(inputState->event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    int w, h; 
                    getWindowSize(app, &w, &h);
                    recalculateViewports(w,h,app);
                    
                    // TODO: use this for reprojection later: float aspect = (float)w / (float)h;
                    //glViewport(0, 0, w / 2, h);
                }
            break;
            case SDL_MOUSEBUTTONUP:
                // A button was released
                inputState->mouseLeftButtonPressed = false;
                
                if(app->cursorSelectionActive)
                {
                    app->deselectCondition = true;
                }
                inputState->mouseDragged = false;
                if
                (
                    app->focusedEntityId != -1 && 
                    app->entities[app->focusedEntityId].uiComponent->type == UITYPE_SLIDER
                )
                    {
                            app->focusedEntityId = -1;
                    }
            break;
            case SDL_MOUSEMOTION:
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
            break;
            default: break;
        }
    }
}

static void handleKeyInput(InputState *inputState, RenderState *renderState, AppState *app){
         
            processKeyInput(inputState->event.key.keysym.sym, app->currentSceneMode);

            SDL_Keycode key = inputState->event.key.keysym.sym;

            switch(key)
            {
                case SDLK_c:
                    randomizeSceneAmbientColor(app);
                break;
                case SDLK_t: {
                    renderState->culling = !renderState->culling;
                    break;
                }
                case SDLK_l: {
                    // Pressing L will write all drawcalls of a frame to disk as a pngs
                    renderState->debugDrawCalls = true;
                    printf("Debug drawcalls enabled\n");
                    break;
                }
                case SDLK_b: {
                    renderState->drawBoundingBoxes = !renderState->drawBoundingBoxes;
                    break;
                }
                case SDLK_ESCAPE: {
                    app->running = 0;
                    break;
                }
                case SDLK_f: {
                    toggleFullscreen(app);
                    break;
                }
                case SDLK_w: {
                    moveCameraForward(app->views.main.camera, app->delta_time);
                    break;
                }
                case SDLK_s: {
                    moveCameraBackward(app->views.main.camera, app->delta_time);
                    break;
                }
                case SDLK_a: {
                    strafeCameraLeft(app->views.main.camera, app->delta_time);
                    break;
                }
                case SDLK_d: {
                    strafeCameraRight(app->views.main.camera, app->delta_time);
                    break;
                }
                case SDLK_o: {
                    setCameraOrthographic(app->views.main.camera);
                    break;
                }
                case SDLK_p: {
                    setCameraPerspective(app->views.main.camera);
                    break;
                }
                case SDLK_i: {
                    toggleCameraMode(app->views.main.camera);
                    break;
                }
                case SDLK_u: {
                    renderState->blinnMode = !renderState->blinnMode;
                    app->showUI = !app->showUI;
                    break;
                }
                case SDLK_g: {
                    renderState->gamma = !renderState->gamma;
                    break;
                }
                case SDLK_h: {
                    renderState->showDepthMap = !renderState->showDepthMap;
                    break;
                }
                default: break;
            }
            
       

}

