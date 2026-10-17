#include "render.h"
#include "ecs-systems.h"
#include "fh-sdl.h"
#include <errno.h>
#include "types.h"
#include "app.h"
#include "ecs.h"
#include "text.h"
#include "utils.h"
#include "camera.h"
#include "api.h"
#include "event.h"
#include "ui.h"

void deleteEntity(Entity* entity);

/**
 * Toggle the childrens visibility between true/false
 * NOTE: Only works in one depth atm, not recursive.
 */
void toggleChildrenVisibility(AppState *app, int entityId) {
    Entity* p = app->entities[entityId].uiComponent->parent;
    if(p == NULL) return;          
    if(p->uiComponent->active && p->uiComponent->childCount > 0) {
        for(int i = 0; i < p->uiComponent->childCount; i++) {
            if(entityId != p->uiComponent->children[i]){
                app->entities[p->uiComponent->children[i]].visible = !app->entities[p->uiComponent->children[i]].visible;
            }
        }
    }
}

void moveCursor(AppState *app, float x){
    app->entities[app->cursorEntityId].transformComponent->position[0] = x;
    app->entities[app->cursorEntityId].transformComponent->modelNeedsUpdate = 1;
}
/**
 * @brief UI input system
 * Handles input on UI elements.
 * TODO: memory leak, we are not deallocating the textCopy memory.
 * TODO: Support remove selected text part
 * TODO: Support copy/paste
 * TODO: Support undo/redo
 * TODO: Support input validation
 * TODO: Support input mask
 * TODO: Support input type (number, email, password etc)
 * TODO: Support input placeholder
 * TODO: Support input focus indicator
 * TODO: Support values change from outside the input field
 * TODO: Support input field disabled state
 * TODO: Home button should put cursor on index 0
 * BUG:  Empty field bugs out on input
 * BUG:  Too far typing to into the right of input field bugs out
 * BUG:  BackSpace in middle of a text should remove left character
 */
void uiInputSystem(AppState *app){
    if(app->focusedEntityId != -1){
         if(app->event.type == SDL_KEYDOWN) {
            const char* key = getKeyName(app->event.key.keysym.sym);
            
            bool isSpaceKey = false;
            bool isSelectionActive = app->cursorSelectionActive;
            
            // Special keys
            if(strcmp(key, "Left") == 0){
               if(isSelectionActive){
                    app->cursorSelectionActive = false;
                    ClosestLetter letter = getCharacterByIndex(app, 0);
                    SDLVector2 sdlVec;
                    sdlVec.x = letter.position.x;
                    sdlVec.y = letter.position.y;
                    UIVector2 uiLetterPos = convertSDLToUI(sdlVec,width,height);
                    moveCursor(app, uiLetterPos.x);
               }else {
                    ClosestLetter cursorLetter = findCharacterUnderCursor(app, width,height);
                    ClosestLetter letter = getCharacterByIndex(app, cursorLetter.characterIndex);
                    SDLVector2 sdlVec;
                    sdlVec.x = letter.position.x - letter.charWidth;
                    sdlVec.y = letter.position.y;
                    UIVector2 uiLetterPos = convertSDLToUI(sdlVec,width,height);
                    moveCursor(app, uiLetterPos.x);
               }
               return;
            }
            if(strcmp(key, "Right") == 0){
                if(isSelectionActive){
                    app->cursorSelectionActive = false;
                    int textLength = strlen(app->entities[app->focusedEntityId].uiComponent->text);
                    ClosestLetter letter = getCharacterByIndex(app, textLength);
                    SDLVector2 sdlVec;
                    sdlVec.x = letter.position.x;
                    sdlVec.y = letter.position.y;
                    UIVector2 uiLetterPos = convertSDLToUI(sdlVec,width,height);
                    moveCursor(app, uiLetterPos.x);
               }else {
                    ClosestLetter letter = findCharacterUnderCursor(app, width,height);
                    SDLVector2 sdlVec;
                    sdlVec.x = letter.position.x + letter.charWidth;
                    sdlVec.y = letter.position.y;
                    UIVector2 uiLetterPos = convertSDLToUI(sdlVec,width,height);
                    moveCursor(app, uiLetterPos.x);
               }
               return;
            }
            if(strcmp(key, "Space") == 0){
                isSpaceKey = true;
            }
            if(strcmp(key, "CapsLock") == 0){
                return;
            }
            if(strcmp(key, "Left Shift") == 0){
                return;
            }
            if(strcmp(key, "Delete") == 0){
                ASSERT(app->focusedEntityId != -1, "No focused entity");

                if(isSelectionActive){
                    ClosestLetter letter = getCharacterByIndex(app, app->cursorTextSelection[0]);
                    SDLVector2 sdlVec;
                    sdlVec.x = letter.position.x;
                    sdlVec.y = letter.position.y;
                    UIVector2 uiLetterPos = convertSDLToUI(sdlVec,width,height);
                    moveCursor(app, uiLetterPos.x);
                    deleteTextRange(app, app->cursorTextSelection[0],app->cursorTextSelection[1]);
                    return;
                }
                handleDeleteButton(app, width,height);
                return;
            }
            if(strcmp(key, "Return") == 0 || strcmp(key, "Escape") == 0){
                if(
                    app->entities[app->focusedEntityId].uiComponent->eventType == EVENT_LIGHT_DIR_X_CHANGED ||
                    app->entities[app->focusedEntityId].uiComponent->eventType == EVENT_LIGHT_DIR_Y_CHANGED 
                ){
                        
                        // Convert string to float 
                        const char *str = app->entities[app->focusedEntityId].uiComponent->text;
                        char *endptr;
                        errno = 0;
                        float value = strtof(str, &endptr);

                        // Cap ranges
                        if(value > 1.0){
                            value = 1.0;
                        }
                        if(value < 0.0){
                            value = 0.0;
                        }

                        // Handle conversion error.
                        if (errno != 0 || endptr == str) {
                            fprintf(stderr, "Conversion error\n");
                        } else {
                            printf("Converted value: %f\n", value);
                            // Send event 
                            TransformPayload transform = { .objectEntityId = app->focusedEntityId,.t = value };
                            Event event = { .type = EVENT_LIGHT_DIR_X_CHANGED, .payload.transform = transform };
                            pushEvent(event);
                        }
                }
                app->focusedEntityId = -1;
                return;
            } 
            if(strcmp(key, "Backspace") == 0){
                if(strlen(app->entities[app->focusedEntityId].uiComponent->text) > 0){

                    // Remove the letter to the left of the cursor.
                    removeCharacter(app, findCharacterUnderCursor(app, width,height).characterIndex-1);

                    // Move cursor one step to the left.
                    UIVector2 mouseCursor;
                    mouseCursor.x = app->entities[app->cursorEntityId].transformComponent->position[0];
                    mouseCursor.y = app->entities[app->cursorEntityId].transformComponent->position[1];
                    SDLVector2 sdlVec = convertUIToSDL(mouseCursor,width,height);
                    ClosestLetter closestLetter = getClosestLetterInText(app, 
                            app->entities[app->focusedEntityId].uiComponent,
                            app->entities[app->focusedEntityId].boundingBoxComponent,
                            sdlVec.x
                    );
                    SDLVector2 closestLetterSDLpos;
                    closestLetterSDLpos.x = closestLetter.position.x;
                    closestLetterSDLpos.y = closestLetter.position.y;
                    UIVector2 uiVec = convertSDLToUI(closestLetterSDLpos,width,height);
                    app->entities[app->cursorEntityId].transformComponent->position[0] = uiVec.x;
                    app->entities[app->cursorEntityId].transformComponent->modelNeedsUpdate = 1;
                }
                return;
            }
                  
            // Any other key pressed
            ASSERT(strlen(app->entities[app->focusedEntityId].uiComponent->text) < 99, "Input field is full");
            
            // TODO: This is not deallocated , memory leak
            char* textCopy = (char*)arena_Alloc(&app->uiArena, 99 * sizeof(char));
            
            // Add pressed key to input field & apply logic uppercase/lowercase/space
            bool doUpperCase = isLeftShiftPressed() ? isCapsLock() ? 0 : 1 : isCapsLock() ? 1 : 0;
            char keyCopy = doUpperCase == 1 ? toUpperCase(key[0]) : toLowerCase(key[0]);
            isSpaceKey ? keyCopy = 32 : keyCopy;
            isLeftShiftPressed() ? keyCopy = specialLeftShiftHandling(keyCopy) : keyCopy;

            // Find closest letter to cursor
            ClosestLetter closestLetter = findCharacterUnderCursor(app, width,height);
        
            // Use closest letter to insert key at the right position
            int j = 0;
            for(int i = 0; i < strlen(app->entities[app->focusedEntityId].uiComponent->text)+1; i++){

                if(i == (closestLetter.characterIndex)){
                    textCopy[i] = keyCopy;
                    j++;
                }

                textCopy[i+j] = app->entities[app->focusedEntityId].uiComponent->text[i];   
            }
            textCopy[strlen(app->entities[app->focusedEntityId].uiComponent->text)+2] = '\0';
            app->entities[app->focusedEntityId].uiComponent->text = textCopy;

            // Move cursor one step to the right
            Character ch = app->characters[(int)keyCopy];
            float advanceCursor = (float)(ch.Advance >> 6) * app->charScale;
            app->entities[app->cursorEntityId].transformComponent->position[0] += advanceCursor;
            app->entities[app->cursorEntityId].transformComponent->modelNeedsUpdate = 1;
         }/* else{
            // I STILL NEED AN ENTITY ID HERE TO THE INPUT FIELD IN THE EVENT
            if(
                app->entities[app->focusedEntityId].uiComponent->event.type == CHANGE_X_DIRECTION 
                && app->entities[app->focusedEntityId].uiComponent->event.fPtr != NULL
            ){
                 snprintf(
                        app->entities[app->focusedEntityId].uiComponent->text,
                        sizeof(app->entities[app->focusedEntityId].uiComponent->text),
                        "%f",
                        app->entities[app->focusedEntityId].uiComponent->event.fPtr
                    );
            }
         } */
         
    }
}

/**
 * @brief Movement system
 * Handles movement update on position,rotation & scale. (Atm only x-axis rotation.)
 * The model matrix that is used for rendering is NOT updated here, but in the modelSystem. ModelSystem uses 
 * the transform values that are updated here to update the model matrix. So this system needs to run before modelsystem.
 * Atm this system is more of a placeholder for movement logic, but will eventually be more complex. 
 * NOTE: Most movement logic is still done in the input function,but will eventually be moved here.
 */
void movementSystem(AppState *app){
    // rotate model logic (temporary)
   // float degrees = 15.5f * app->delta_time;
   // float radians = degrees * M_PI / 180.0f;

    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(app->entities[i].alive == 1) {
            
           if(app->entities[i].transformComponent->active == 1){
                // Do movement logic here:
             
                // Example of movement logic: Rotate on y-axis on all entities that are not ui (temporary)
                if(app->entities[i].uiComponent->active == 1){
                  //  printf("entity %d \n", i);
                    //printf("confirmed active ui\n");
                   /*  if(isPointInsideRect(app->entities[i].uiComponent->boundingBox, (vec2){ app->event.motion.x, app->event.motion.y})){
                    app->entities[i].transformComponent->scale[0] += 1.5f;
                    app->entities[i].transformComponent->modelNeedsUpdate = 1; */
                     /*    printf("bb x %d ", app->entities[i].uiComponent->boundingBox.x);
                        printf("bb y %d ", app->entities[i].uiComponent->boundingBox.y);
                        printf("bb width %d ", app->entities[i].uiComponent->boundingBox.width);
                        printf("bb height %d ", app->entities[i].uiComponent->boundingBox.height); */
                        //app->entities[i].transformComponent->rotation[1] = radians;
                        //app->entities[i].transformComponent->modelNeedsUpdate = 1;
                   // }
                }else if(app->entities[i].meshComponent->active == 1 && app->entities[i].id == 0){
                       //printf("entity %d \n", i);
                       //float offset = 20.0 * sin(0.5 * app->delta_time);
                  //  app->entities[i].transformComponent->rotation[1] = app->delta_time;
                  //  app->entities[i].transformComponent->modelNeedsUpdate = 1;
                
                }
                if(app->entities[i].lightComponent->active == 1 && app->entities[i].id == app->lights[0].entityId){
                    if(app->focusedEntityId != -1 && app->entities[app->focusedEntityId].uiComponent->active){

                    /*   if(
                        app->entities[app->focusedEntityId].uiComponent->eventType == EVENT_REQUEST_LIGHT_DIR_CHANGE 
                        && app->entities[app->focusedEntityId].uiComponent->type == UITYPE_SLIDER
                      ){
                        printf("SHOULD UPDATE using updated sliderValue, value: %f \n",app->entities[app->focusedEntityId].uiComponent->sliderValue * 100);
                        app->entities[i].lightComponent->direction[0] = app->entities[app->focusedEntityId].uiComponent->sliderValue * 100;
                        app->entities[i].transformComponent->modelNeedsUpdate = 1;
                      }

                      if(app->entities[app->focusedEntityId].uiComponent->eventType == EVENT_REQUEST_LIGHT_DIR_CHANGE){
                        float value = app->entities[app->focusedEntityId].uiComponent->sliderValue;
                        printf("y value %f \n",value);
                        app->entities[i].lightComponent->direction[1] = value * 10.0;
                        app->entities[i].transformComponent->modelNeedsUpdate = 1;
                      }

                      if(app->entities[app->focusedEntityId].uiComponent->eventType == EVENT_REQUEST_LIGHT_DIR_CHANGE){
                        float value = app->entities[app->focusedEntityId].uiComponent->sliderValue;
                        printf("z value %f \n",value);
                        app->entities[i].lightComponent->direction[2] = value * 10.0;
                        app->entities[i].transformComponent->modelNeedsUpdate = 1;
                      } */
                    
                    // app->entities[i].transformComponent->rotation[1] = radians;
                       // float offset = 20.0 * sin(0.5 * app->delta_time);
                        //printf("offset %f\n", offset);
                      /*   app->entities[i].transformComponent->position[0] = offset * multiplicator;
                        app->entities[i].transformComponent->position[2] = offset * multiplicator; */
                    }else {
                      //  printf("no slider action\n");
                    }
                   
                }
                //app->entities[i].transformComponent->rotation[1] += radians; //<- This is an example of acceleration.
           }
        }
    }
}

/**
 * @brief Camera system
 * Handles camera update.
 * Atm we are not handling everything about the camera here, just the update of projection & view.
 * Movement is handled in the input function, but will eventually be moved here.
 */
void cameraSystem(AppState *app){
    updateCamera(app, app->views.main.camera);
    updateCamera(app, app->views.ui.camera);
}

/**
 * @brief UI positionSystem
 * Responsible for setting the position of the UI elements in "UI" space. 
 * UI space is top left corner of the screen is [0.0,0.0], bottom right is [width,height].
 * Newly created UI elements is spawned in center of the screen [width/2, height/2] 
 * ¨ystem then is responsible for 
 * setting the position to top left corner + the requested position.
 */
void uiPositionSystem(AppState *app){
    for(int i = 0; i < MAX_ENTITIES; i++) {
   
        if(
            app->entities[i].alive == 1 
        && app->entities[i].uiComponent->active 
        && app->entities[i].transformComponent->active 
        && app->entities[i].uiComponent->uiNeedsUpdate 
        && app->cursorEntityId != i
        && app->entities[i].boundingBoxComponent->active
        ) {
        
         
                // Goal here: Position element in "UI" space, where start 0,0 is the center of the ui-viewport screen. 
                // We will position the element to top left corner of the screen and treat this as 0,0 instead.
                // If position or scale changes, this calculation won't be correct anymore.
                // This calculation only works for elements that are spawned in the center of the screen.
                
                // Position of element on spawn:
                float ui_viewport_half_width  = (float)app->views.ui.rect.width  / 2; 
                float ui_viewport_half_height = (float)app->views.ui.rect.height / 2;
              //  printf("ui_viewport_half_width %f\n", ui_viewport_half_width);
              //  printf("ui_viewport_half_height %f\n", ui_viewport_half_height);
                
                // Half scale of element
                float scaleInPixelsX = app->entities[i].transformComponent->scale[0]; 
                float scaleInPixelsY = app->entities[i].transformComponent->scale[1]; /// app->unitScale;
              //  printf("scaleInPixelsX %f\n", scaleInPixelsX);
              //  printf("scaleInPixelsY %f\n", scaleInPixelsY);

                // TODO: rotation
            
                // position of element
                float requested_pos_x = app->entities[i].transformComponent->position[0];
                float requested_pos_y = app->entities[i].transformComponent->position[1];

              //  printf("requested_pos_x %f\n", requested_pos_x);
              //  printf("requested_pos_y %f\n", requested_pos_y);

              //  printf("before %f %f\n", app->entities[i].transformComponent->position[0], app->entities[i].transformComponent->position[1]);
        
                // move element to upper left corner and then add requested position.
               app->entities[i].transformComponent->position[0] = (float)(ui_viewport_half_width - (scaleInPixelsX * 0.5) - requested_pos_x) * -1.0; 
               app->entities[i].transformComponent->position[1] = (float)(ui_viewport_half_height - (scaleInPixelsY * 0.5)) - requested_pos_y * 1.0;
             //  printf("final pos: %f %f\n", app->entities[i].transformComponent->position[0], app->entities[i].transformComponent->position[1]);
                
                // Bounding box
                //app->entities[i].uiComponent->boundingBox.x = requested_pos_x;
              /*   app->entities[i].boundingBoxComponent->boundingBox.min[0] = requested_pos_x;
                app->entities[i].boundingBoxComponent->boundingBox.min[1] = requested_pos_y;
                app->entities[i].boundingBoxComponent->boundingBox.max[0] = app->entities[i].transformComponent->scale[0];
                app->entities[i].boundingBoxComponent->boundingBox.max[1] = app->entities[i].transformComponent->scale[1]; */
                
               /*  printf("bounding box x %d\n", app->entities[i].uiComponent->boundingBox.x);
                printf("bounding box y %d\n", app->entities[i].uiComponent->boundingBox.y);
                printf("bounding box width %d\n", app->entities[i].uiComponent->boundingBox.width);
                printf("bounding box height %d\n", app->entities[i].uiComponent->boundingBox.height);  
                printf("entity id %d\n", app->entities[i].id);
                printf("bb entity to update %d\n", app->entities[i].uiComponent->boundingBoxEntityId); */
                
                // TODO: I dont think this is necessary, we should be able to direct access with i.
                // Find the bounding box entity and update with new values
                for(int j = 0; j < MAX_ENTITIES; j++) {
                    if(app->entities[j].alive == 1 && app->entities[i].uiComponent->boundingBoxEntityId == app->entities[j].id) { 
                        app->entities[j].transformComponent->position[0] = app->entities[i].transformComponent->position[0];
                        app->entities[j].transformComponent->position[1] = app->entities[i].transformComponent->position[1];
                        app->entities[j].transformComponent->scale[0] = app->entities[i].transformComponent->scale[0];
                        app->entities[j].transformComponent->scale[1] = app->entities[i].transformComponent->scale[1];
                        app->entities[j].transformComponent->modelNeedsUpdate = 1;
                    }
                }
                app->entities[i].uiComponent->uiNeedsUpdate = 0;
        }
    }
}

void hoverAndClickSystem(AppState *app){
    int newCursor = SDL_SYSTEM_CURSOR_ARROW;
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(app->entities[i].alive == 1) {
            if(app->entities[i].transformComponent->active == 1 && app->entities[i].uiComponent->active == 1 && app->entities[i].boundingBoxComponent->active == 1 && app->entities[i].materialComponent->active == 1){

                    // UI text and non-UI is not handled by this system.
                    if(app->entities[i].uiComponent->type == UITYPE_TEXT || app->entities[i].uiComponent->type == UITYPE_NONE){
                        continue;
                    }

                    if(
                        app->views.ui.isMousePointerWithin && 
                        isPointInsideBoundingBox(app->entities[i].boundingBoxComponent->boundingBox, (vec2){ app->mouseXpos, app->mouseYpos})
                    ){

                        // Left Click or just hover?
                        if(app->mouseLeftButtonPressed && app->prevMouseLeftDown == false){
                            if(!app->entities[i].uiComponent->clicked && app->entities[i].uiComponent->type == UITYPE_BUTTON){
                                if(app->entities[i].uiComponent->eventType == EVENT_TOGGLE_UI_PANEL){
                                    if(!app->entities[i].uiComponent->parent){
                                        printf("no parent to toggle panel on \n");
                                        continue;
                                    }
                                    printf("trying to toggle panel \n");
                                    toggleChildrenVisibility(app, i); // Refactor to event?
                               }
                            }
                  
                            app->entities[i].uiComponent->clicked = 1;
                        } else {
                   
                            app->entities[i].uiComponent->hovered = 1;
                            app->entities[i].uiComponent->clicked = 0;
                        }

                        // Hover effect
                        if(app->entities[i].uiComponent->hovered == 1){
                            if(app->entities[i].uiComponent->type == UITYPE_INPUT){
                                newCursor = SDL_SYSTEM_CURSOR_IBEAM;
                            }
                            if(
                               app->entities[i].uiComponent->type == UITYPE_BUTTON 
                            || app->entities[i].uiComponent->type == UITYPE_SLIDER
                            || app->entities[i].uiComponent->type == UITYPE_CHECKBOX
                            ){
                                newCursor = SDL_SYSTEM_CURSOR_HAND;
                            }
                            if(
                                strlen(app->entities[i].uiComponent->text) > 0 
                                || app->entities[i].uiComponent->type == UITYPE_SLIDER
                                || app->entities[i].uiComponent->type == UITYPE_CHECKBOX
                            ){
                        
                               // Appearance changes when hovered
                               app->entities[i].materialComponent->diffuseMapOpacity = app->entities[i].materialComponent->diffuseMapOpacity * 0.5f;
                               app->entities[i].materialComponent->diffuse.r = 0.0f;
                               app->entities[i].materialComponent->diffuse.g = 0.5f;
                            }
                        }

                        // Appearance changes when mouse down (instead of hovered it should be focusedEntity ?)
                        if(app->entities[i].uiComponent->hovered && app->mouseLeftButtonPressed){
                            app->entities[i].materialComponent->diffuseMapOpacity = app->entities[i].materialComponent->diffuseMapOpacity * 0.5f;
                            app->entities[i].materialComponent->diffuse.r = 0.5f;
                            app->entities[i].materialComponent->diffuse.g = 0.0f;
                        }

                        if(app->entities[i].uiComponent->clicked == 1){
                           
                            if(app->entities[i].uiComponent->type == UITYPE_INPUT || app->entities[i].uiComponent->type == UITYPE_SLIDER){
                                app->focusedEntityId = app->entities[i].id;
                            }
                        
                            // Actions when clicked
                            if(app->entities[i].uiComponent->eventType == EVENT_TOGGLE_CAST_SHADOW) printf("toggle cast shadow \n");
                        }
                    
                    } else {
                         if(strlen(app->entities[i].uiComponent->text) > 0){
                               // printf("no action,disable actions\n");
                            } 
                        Material *material = getMaterial(app, app->entities[i].materialComponent->materialIndex);
                        app->entities[i].uiComponent->hovered = 0;
                        app->entities[i].uiComponent->clicked = 0;
                        app->entities[i].materialComponent->diffuseMapOpacity = material->diffuseMapOpacity;
                        app->entities[i].materialComponent->diffuse.r = material->diffuse.r;
                        app->entities[i].materialComponent->diffuse.g = material->diffuse.g;
                        app->entities[i].materialComponent->diffuse.b = material->diffuse.b;
                    }
            }
        }
    } 
    changeCursor(newCursor);
}

void uiSliderSystem(AppState *app){
    if(
        app->focusedEntityId != -1 
        && app->entities[app->focusedEntityId].uiComponent->type == UITYPE_SLIDER
        && app->mouseDragged
        ){
          
            // Draggable range
            Entity* focusedEntity = &app->entities[app->focusedEntityId];
            Entity* sliderRangeEntity = &app->entities[focusedEntity->uiComponent->sliderRangeEntityId];
         
            float minRange = sliderRangeEntity->boundingBoxComponent->boundingBox.min[0]; 
            float maxRange = sliderRangeEntity->boundingBoxComponent->boundingBox.max[0]; 
            float rangeX   = focusedEntity->uiComponent->sliderRange;  
            float newMouseX = (float)app->mouseXpos;
            float t = (newMouseX - minRange) / rangeX;
            if(t > 1.0f){
                t = 1.0f;
                printf("out of range \n");
            }
            if(t <= 0.0f){
                t = 0.0f;
                printf("out of range \n");
            }
          
            // If within draggable range
            if(newMouseX > minRange && newMouseX < maxRange){
               if(focusedEntity->uiComponent->eventType == EVENT_LIGHT_DIR_X_CHANGED){
                 TransformPayload transform = { .objectEntityId = app->focusedEntityId,.t = t };
                 Event event = { .type = EVENT_LIGHT_DIR_X_CHANGED, .payload.transform = transform };
                 pushEvent(event);
               } 
            } 
    }
}

void textCursorSystem(AppState *app){
    
    if(app->focusedEntityId != -1){
        if(app->entities[app->focusedEntityId].uiComponent->type == UITYPE_SLIDER){
            return;
        }
        bool cursorExist = app->cursorEntityId != -1;
     //   bool onBlur = !isPointInsideRect(app->entities[app->focusedEntityId].uiComponent->boundingBox, (vec2){ app->mouseXpos, app->mouseYpos});

        if(cursorExist){
            
            if(app->mouseLeftButtonPressed){
                
                // click and drag to select text
                if(app->mouseDragged){
                   
                    // Find closest letter to cursor on dragstart
                    if(app->cursorDragStart == -1.0f){
                        ClosestLetter closestStartLetter = getClosestLetterInText(app, 
                            app->entities[app->focusedEntityId].uiComponent,
                            app->entities[app->focusedEntityId].boundingBoxComponent, 
                            app->mouseXpos);
                        addIndexToCursorTextSelection(app, (unsigned int)closestStartLetter.characterIndex);
                        SDLVector2 sdlVec;
                        sdlVec.x = closestStartLetter.position.x;
                        sdlVec.y = closestStartLetter.position.y;         
                        UIVector2 uiVec = convertSDLToUI(sdlVec,width,height);
                        moveCursor(app, uiVec.x);
                        app->cursorDragStart = uiVec.x;
                    }

                    // Find closest letter to cursor on dragend
                    ClosestLetter closestEndLetter = getClosestLetterInText(app, 
                        app->entities[app->focusedEntityId].uiComponent,
                        app->entities[app->focusedEntityId].boundingBoxComponent, 
                        app->mouseXpos);
                    addIndexToCursorTextSelection(app, (unsigned int)closestEndLetter.characterIndex);
                    SDLVector2 sdlVec;
                    sdlVec.x = closestEndLetter.position.x;
                    sdlVec.y = closestEndLetter.position.y;         
                    UIVector2 uiVec = convertSDLToUI(sdlVec,width,height);
                    moveCursor(app, uiVec.x);
                    
                    // Draw selection
                    app->cursorSelectionActive = true;
                   // float rectangleStartPos = app->cursorDragStart - (uiVec.x * 0.5);
                    float selectionWidth = uiVec.x - app->cursorDragStart;

                    // Set cursor to be a new width & position 
                    app->entities[app->cursorEntityId].transformComponent->position[0] = app->cursorDragStart + (selectionWidth * 0.5);
                    app->entities[app->cursorEntityId].transformComponent->scale[0] = selectionWidth;
                    app->entities[app->cursorEntityId].transformComponent->modelNeedsUpdate = 1;

                }else if(app->cursorDragStart){
                    app->cursorDragStart = -1.0f;
                }
                
                // Double click select all text
                if(app->mouseDoubleClick == 1 && app->cursorSelectionActive == false){
                    selectAllText(app, width,height);
                    app->mouseDoubleClick = 0;
                }

                // Single click, place cursor where mouse clicked
                if(app->mouseDoubleClick == 0 && app->cursorSelectionActive == false){

                    ClosestLetter closestLetter = getClosestLetterInText(app, 
                        app->entities[app->focusedEntityId].uiComponent, 
                        app->entities[app->focusedEntityId].boundingBoxComponent, 
                        app->mouseXpos);
                    SDLVector2 sdlVec;
                    sdlVec.x = closestLetter.position.x;
                    sdlVec.y = closestLetter.position.y;         
                    UIVector2 uiVec = convertSDLToUI(sdlVec,width,height);
                    moveCursor(app, uiVec.x);
                }

                // Single click, deselect text
                if(app->cursorSelectionActive && app->deselectCondition){
                    app->cursorSelectionActive = false;
                    app->deselectCondition = false;
                }
                
            }

            // Blink cursor logic
            if(app->entities[app->cursorEntityId].uiComponent->active == 1){
                if(app->delta_time - app->cursorBlinkTime > 0.6f){
                    app->cursorBlinkTime = app->delta_time;
                    app->entities[app->cursorEntityId].uiComponent->active = 0;
                }
            }else {
                if(app->delta_time - app->cursorBlinkTime > 0.6f){
                    app->cursorBlinkTime = app->delta_time;
                    app->entities[app->cursorEntityId].uiComponent->active = 1;
                }
            }
            if(app->cursorSelectionActive){
                app->entities[app->cursorEntityId].uiComponent->active = 1;
            }else{
                
                // Set mouse cursor scale to normal scale
                app->entities[app->cursorEntityId].transformComponent->scale[0] = 1.5f;
                
            }
        } 
  
        // Create cursor
        if(!cursorExist){
            ClosestLetter closestLetter = getClosestLetterInText(app, 
                app->entities[app->focusedEntityId].uiComponent, 
                app->entities[app->focusedEntityId].boundingBoxComponent, 
                app->mouseXpos
            );
            printf("closest letter position x %f\n", closestLetter.position.x);
            printf("closest letter position y %f\n", closestLetter.position.y);
            SDLVector2 sdlVec;
            sdlVec.x = closestLetter.position.x;
            sdlVec.y = closestLetter.position.y;         
            UIVector2 uiVec = convertSDLToUI(sdlVec,width,height);
            app->cursorEntityId = ui_createRectangle(app, app->materials[0], (vec3){uiVec.x, uiVec.y, 2.0f}, (vec3){1.5f, (float)app->fontSize*0.75f, 5.0f}, (vec3){0.0f, 0.0f, 0.0f},NULL);
        }
        
    }else{

        // When we are not focused on an input field, we should remove the cursor.
        if(app->cursorEntityId != -1){
            deleteEntity(&app->entities[app->cursorEntityId]);
            app->cursorEntityId = -1;
        }
    }
}

/**
 * @brief Model system
 * Handles model update.
 * Atm we are not handling everything about the model here, just the update of model matrix.
 * Movement is handled in the input function, but will eventually be moved here or to a separate movement-system. 
 * Perhaps the model matrix update will move there aswell or maybe be handled in some kind of transform hierarchy system, 
 * since groups & hierarchy of entities should be something we would need when we try to build more complex scenes.
 * NOTE: Atm this is more of a placeholder for model matrix update logic, since we only handle rotation on x-axis and not even handling scale change. 
 * So atm only works with rigid body transforms.
 */
void modelSystem(AppState *app){
    // Look for transform needs update flag & update/recalc model matrix if needed.
     for(int i = 0; i < MAX_ENTITIES; i++) {
        if(app->entities[i].alive == 1) {
           if(app->entities[i].transformComponent->modelNeedsUpdate == 1) {
                    // create transformations
                    mat4x4 model;
                    mat4x4_identity(model);

                    // set model position
                    mat4x4_translate(model, app->entities[i].transformComponent->position[0],app->entities[i].transformComponent->position[1],app->entities[i].transformComponent->position[2]);

                    // set model scale
                    mat4x4_scale_aniso(model, (const float (*)[4])model, app->entities[i].transformComponent->scale[0],app->entities[i].transformComponent->scale[1],app->entities[i].transformComponent->scale[2]);
                
                    // rotate model on x, y, and z axes
                    // The cast on model tells the compiler that you're aware of the 
                    // const requirement and that you're promising not to modify the model matrix.
                    mat4x4 rotatedModelX, rotatedModelY, rotatedModelZ;
                    mat4x4_rotate_X(rotatedModelX, (const float (*)[4])model, app->entities[i].transformComponent->rotation[0]);
                    mat4x4_rotate_Y(rotatedModelY, (const float (*)[4])rotatedModelX, app->entities[i].transformComponent->rotation[1]);
                    mat4x4_rotate_Z(rotatedModelZ, (const float (*)[4])rotatedModelY, app->entities[i].transformComponent->rotation[2]);

                    // Copy the rotated model matrix to the transform component
                    memcpy(app->entities[i].transformComponent->transform, rotatedModelZ, sizeof(mat4x4));

                    app->entities[i].transformComponent->modelNeedsUpdate = 0;
           }
          }}
}

void debugSystem(RenderState *renderState){
    
    // Turn off debug draw calls, we only want one frame of drawcalls saved.
    if(renderState->debugDrawCalls){
        renderState->debugDrawCalls = false;
    }
       
}

void uiCheckboxSystem(RenderState *renderState, AppState *app){

    for(int i = 0; i < MAX_ENTITIES; i++){
        if(
            app->entities[i].alive 
            && app->entities[i].visible 
            && app->entities[i].uiComponent->active
            && app->entities[i].uiComponent->type == UITYPE_CHECKBOX
            && app->entities[i].uiComponent->clicked
            ){
                if(app->entities[i].uiComponent->checked){
                    app->entities[i].uiComponent->checked = false;
                    app->entities[app->entities[i].uiComponent->checkedEntityId].materialComponent->diffuse.g = flatColorUiDarkGrayMat.diffuse.g;
                    printf("unchecked \n");
                   
                }else {
                    app->entities[i].uiComponent->checked = true;
                    app->entities[app->entities[i].uiComponent->checkedEntityId].materialComponent->diffuse.g = 1.0;
                    renderState->shadows = true;
                    printf("checked \n");
                  
                }
            }
    }

    static bool firstRun = true;

    if (firstRun) {
        for(int i = 0; i < MAX_ENTITIES; i++){
            if(
                app->entities[i].alive 
                && app->entities[i].visible 
                && app->entities[i].uiComponent->active
                && app->entities[i].uiComponent->type == UITYPE_CHECKBOX
            ){
                if(app->entities[i].uiComponent->checked){
                    app->entities[app->entities[i].uiComponent->checkedEntityId].materialComponent->diffuse.g = 1.0;
                }else {
                    app->entities[app->entities[i].uiComponent->checkedEntityId].materialComponent->diffuse.g = flatColorUiDarkGrayMat.diffuse.g;
                   
                }
            }
        }

        firstRun = false;
    } 
}
