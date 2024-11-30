#include "ecs-systems.h"
#include "types.h"
#include "globals.h"
#include "ecs.h"
#include "text.h"
#include "utils.h"

void moveCursor(float x){
    globals.entities[globals.cursorEntityId].transformComponent->position[0] += x;
    globals.entities[globals.cursorEntityId].transformComponent->modelNeedsUpdate = 1;
}
/**
 * @brief UI input system
 * Handles input on UI elements.
 * TODO: memory leak, we are not deallocating the textCopy memory.
 * TODO: Support !?#
 * TODO: Support arrow keys for moving cursor
 * TODO: Support selecting text
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
 * BUG:  Empty field bugs out on input
 * BUG:  Too far typing to into the right of input field bugs out
 * BUG:  BackSpace in middle of a text should remove left character
 */
void uiInputSystem(){
    if(globals.focusedEntityId != -1){
         if(globals.event.type == SDL_KEYDOWN) {
            char* key = SDL_GetKeyName(globals.event.key.keysym.sym);
            
            bool isSpaceKey = false;
            
            // Special keys
            if(strcmp(key, "Left") == 0){
                moveCursor(findCharacterUnderCursor(width,height).charWidth*-1.0);
                return;
            }
            if(strcmp(key, "Right") == 0){
                moveCursor(findCharacterUnderCursor(width,height).charWidth);
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
                ASSERT(globals.focusedEntityId != -1, "No focused entity");
                handleDeleteButton(width,height);
                return;
            }
            if(strcmp(key, "Return") == 0 || strcmp(key, "Escape") == 0){
                globals.focusedEntityId = -1;
                return;
            } 
            if(strcmp(key, "Backspace") == 0){
                if(strlen(globals.entities[globals.focusedEntityId].uiComponent->text) > 0){

                    // Remove the letter to the left of the cursor
                    removeCharacter(findCharacterUnderCursor(width,height).characterIndex-1);

                    // Move cursor one step to the left
                    Vector2 sdlVec = convertUIToSDL(
                        globals.entities[globals.cursorEntityId].transformComponent->position[0], 
                        globals.entities[globals.cursorEntityId].transformComponent->position[1],
                        width,height);
                    ClosestLetter closestLetter = getClosestLetterInText(
                            globals.entities[globals.focusedEntityId].uiComponent,
                            sdlVec.x
                    );
                    Vector2 uiVec = convertSDLToUI(closestLetter.position.x, closestLetter.position.y,width,height);
                    globals.entities[globals.cursorEntityId].transformComponent->position[0] = uiVec.x;
                    globals.entities[globals.cursorEntityId].transformComponent->modelNeedsUpdate = 1;
                }
                return;
            }
                  
            // Any other key pressed
            ASSERT(strlen(globals.entities[globals.focusedEntityId].uiComponent->text) < 99, "Input field is full");
            
            // TODO: This is not deallocated , memory leak
            char* textCopy = (char*)arena_Alloc(&globals.uiArena, 99 * sizeof(char));
            
            // Add pressed key to input field & apply logic uppercase/lowercase/space
            bool doUpperCase = isLeftShiftPressed() ? isCapsLock() ? 0 : 1 : isCapsLock() ? 1 : 0;
            char keyCopy = doUpperCase == 1 ? toUpperCase(key[0]) : toLowerCase(key[0]);
            isSpaceKey ? keyCopy = 32 : keyCopy;
            isLeftShiftPressed() ? keyCopy = specialLeftShiftHandling(keyCopy) : keyCopy;

            // Find closest letter to cursor
            ClosestLetter closestLetter = findCharacterUnderCursor(width,height);
        
            // Use closest letter to insert key at the right position
            int j = 0;
            for(int i = 0; i < strlen(globals.entities[globals.focusedEntityId].uiComponent->text)+1; i++){

                if(i == (closestLetter.characterIndex)){
                    textCopy[i] = keyCopy;
                    j++;
                }

                textCopy[i+j] = globals.entities[globals.focusedEntityId].uiComponent->text[i];   
            }
            textCopy[strlen(globals.entities[globals.focusedEntityId].uiComponent->text)+2] = '\0';
            globals.entities[globals.focusedEntityId].uiComponent->text = textCopy;

            // Move cursor one step to the right
            Character ch = globals.characters[keyCopy];
            float advanceCursor = (float)(ch.Advance >> 6) * globals.charScale;
            globals.entities[globals.cursorEntityId].transformComponent->position[0] += advanceCursor;
            globals.entities[globals.cursorEntityId].transformComponent->modelNeedsUpdate = 1;
         }
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
void movementSystem(){
    // rotate model logic (temporary)
    float degrees = 15.5f * globals.delta_time;
   // float radians = degrees * M_PI / 180.0f;

    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            
           if(globals.entities[i].transformComponent->active == 1){
                // Do movement logic here:
                
                // Example of movement logic: Rotate on y-axis on all entities that are not ui (temporary)
                if(globals.entities[i].uiComponent->active == 1){
                  //  printf("entity %d \n", i);
                    //printf("confirmed active ui\n");
                   /*  if(isPointInsideRect(globals.entities[i].uiComponent->boundingBox, (vec2){ globals.event.motion.x, globals.event.motion.y})){
                    globals.entities[i].transformComponent->scale[0] += 1.5f;
                    globals.entities[i].transformComponent->modelNeedsUpdate = 1; */
                     /*    printf("bb x %d ", globals.entities[i].uiComponent->boundingBox.x);
                        printf("bb y %d ", globals.entities[i].uiComponent->boundingBox.y);
                        printf("bb width %d ", globals.entities[i].uiComponent->boundingBox.width);
                        printf("bb height %d ", globals.entities[i].uiComponent->boundingBox.height); */
                        //globals.entities[i].transformComponent->rotation[1] = radians;
                        //globals.entities[i].transformComponent->modelNeedsUpdate = 1;
                   // }
                }
                if(globals.entities[i].lightComponent->active == 1){
                   // globals.entities[i].transformComponent->rotation[1] = radians;
                    globals.entities[i].transformComponent->modelNeedsUpdate = 1;
                   // float offset = 2.0 * sin(1.0 * globals.delta_time);
                  //  globals.entities[i].lightComponent->direction[0] = offset;
                    //printf("offset %f\n", offset);
                   // globals.entities[i].transformComponent->position[0] = offset;
                }
                //globals.entities[i].transformComponent->rotation[1] += radians; //<- This is an example of acceleration.
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
void cameraSystem(){
  
    updateCamera(globals.views.main.camera);
    updateCamera(globals.views.ui.camera);
}

/**
 * @brief UI system
 * Responsible for setting the position of the UI elements in "UI" space. 
 * UI space is top left corner of the screen is [0.0,0.0], bottom right is [width,height].
 * Newly created UI elements is spawned in center of the screen [width/2, height/2] uiSystem then is responsible for 
 * setting the position to top left corner + the requested position.
 */
void uiSystem(){
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(
            globals.entities[i].alive == 1 
        && globals.entities[i].uiComponent->active 
        && globals.entities[i].transformComponent->active 
        && globals.entities[i].uiComponent->uiNeedsUpdate 
        && globals.cursorEntityId != i
        ) {
                
                // Goal here: Position element in "UI" space, where start 0,0 is the center of the ui-viewport screen. 
                // We will position the element to top left corner of the screen and treat this as 0,0 instead.
                // If position or scale changes, this calculation won't be correct anymore.
                // This calculation only works for elements that are spawned in the center of the screen.
                
                // Position of element on spawn:
                float ui_viewport_half_width = (float)globals.views.ui.rect.width / 2; 
                float ui_viewport_half_height = (float)globals.views.ui.rect.height / 2;
              //  printf("ui_viewport_half_width %f\n", ui_viewport_half_width);
              //  printf("ui_viewport_half_height %f\n", ui_viewport_half_height);
                
                // Half scale of element
                float scaleInPixelsX = globals.entities[i].transformComponent->scale[0]; 
                float scaleInPixelsY = globals.entities[i].transformComponent->scale[1]; /// globals.unitScale;
              //  printf("scaleInPixelsX %f\n", scaleInPixelsX);
              //  printf("scaleInPixelsY %f\n", scaleInPixelsY);

                // TODO: rotation
            
                // position of element
                float requested_pos_x = globals.entities[i].transformComponent->position[0];
                float requested_pos_y = globals.entities[i].transformComponent->position[1];

              //  printf("requested_pos_x %f\n", requested_pos_x);
              //  printf("requested_pos_y %f\n", requested_pos_y);

              //  printf("before %f %f\n", globals.entities[i].transformComponent->position[0], globals.entities[i].transformComponent->position[1]);
        
                // move element to upper left corner and then add requested position.
               globals.entities[i].transformComponent->position[0] = (float)(ui_viewport_half_width - (scaleInPixelsX * 0.5) - requested_pos_x) * -1.0; 
               globals.entities[i].transformComponent->position[1] =(float)(ui_viewport_half_height - (scaleInPixelsY * 0.5)) - requested_pos_y * 1.0;
             //  printf("final pos: %f %f\n", globals.entities[i].transformComponent->position[0], globals.entities[i].transformComponent->position[1]);
                
                // Bounding box
                globals.entities[i].uiComponent->boundingBox.x = requested_pos_x;
                globals.entities[i].uiComponent->boundingBox.y = requested_pos_y;
                globals.entities[i].uiComponent->boundingBox.width = globals.entities[i].transformComponent->scale[0];
                globals.entities[i].uiComponent->boundingBox.height = globals.entities[i].transformComponent->scale[1];
                
               /*  printf("bounding box x %d\n", globals.entities[i].uiComponent->boundingBox.x);
                printf("bounding box y %d\n", globals.entities[i].uiComponent->boundingBox.y);
                printf("bounding box width %d\n", globals.entities[i].uiComponent->boundingBox.width);
                printf("bounding box height %d\n", globals.entities[i].uiComponent->boundingBox.height);  
                printf("entity id %d\n", globals.entities[i].id);
                printf("bb entity to update %d\n", globals.entities[i].uiComponent->boundingBoxEntityId); */
                

                // Find the bounding box entity and update with new values
                for(int j = 0; j < MAX_ENTITIES; j++) {
                    if(globals.entities[j].alive == 1 && globals.entities[i].uiComponent->boundingBoxEntityId == globals.entities[j].id) { 
                        globals.entities[j].transformComponent->position[0] = globals.entities[i].transformComponent->position[0];
                        globals.entities[j].transformComponent->position[1] = globals.entities[i].transformComponent->position[1];
                        globals.entities[j].transformComponent->scale[0] = globals.entities[i].transformComponent->scale[0];
                        globals.entities[j].transformComponent->scale[1] = globals.entities[i].transformComponent->scale[1];
                        globals.entities[j].transformComponent->modelNeedsUpdate = 1;
                    }
                }
                globals.entities[i].uiComponent->uiNeedsUpdate = 0;
        }
    }
}

void hoverAndClickSystem(){
    int newCursor = SDL_SYSTEM_CURSOR_ARROW;
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
            if(globals.entities[i].transformComponent->active == 1 && globals.entities[i].uiComponent->active == 1){
                   
                    if(
                        globals.views.ui.isMousePointerWithin && 
                        isPointInsideRect(globals.entities[i].uiComponent->boundingBox, (vec2){ globals.mouseXpos, globals.mouseYpos})
                    ){
                       
                        // Left Click or just hover?
                        if(globals.mouseLeftButtonPressed){
                            if(strlen(globals.entities[i].uiComponent->text) > 0){
                               // printf("clicked\n");
                            }
                          
                            globals.entities[i].uiComponent->clicked = 1;
                        } else {
                            
                             if(strlen(globals.entities[i].uiComponent->text) > 0){
                              //  printf("hovered entity with id: %d\n", globals.entities[i].id);
                            }
                            globals.entities[i].uiComponent->hovered = 1;
                            globals.entities[i].uiComponent->clicked = 0;
                        }

                        // Hover effect
                        if(globals.entities[i].uiComponent->hovered == 1){
                            if(globals.entities[i].uiComponent->type == UITYPE_INPUT){
                                newCursor = SDL_SYSTEM_CURSOR_IBEAM;
                            }
                            if(globals.entities[i].uiComponent->type == UITYPE_BUTTON){
                                newCursor = SDL_SYSTEM_CURSOR_HAND;
                            }
                            if(strlen(globals.entities[i].uiComponent->text) > 0){
                              // printf("hovered changes done on entity with id %d\n", globals.entities[i].id);
                                // Appearance changes when hovered
                               globals.entities[i].materialComponent->diffuseMapOpacity = globals.entities[i].materialComponent->diffuseMapOpacity * 0.5f;
                               globals.entities[i].materialComponent->diffuse.r = 0.0f;
                               globals.entities[i].materialComponent->diffuse.g = 0.5f;
                            }
                        }

                        if(globals.entities[i].uiComponent->clicked == 1){
                           // printf("clicked entity with id: %d\n", globals.entities[i].id);
                            if(globals.entities[i].uiComponent->type == UITYPE_INPUT){
                                globals.focusedEntityId = globals.entities[i].id;
                               // printf("input field on entity with id %d clicked\n", globals.entities[i].id);
                            }else {
                                // If clicked ui element is not an input field
                                //if(globals.focusedEntityId != globals.entities[i].id)
                                    // Reset focused entity if there was one, same as onBlur.
                                   // printf("onblur/defocusing entity id %d\n", globals.focusedEntityId);
                                  //  globals.focusedEntityId = -1;
                                
                            }
                            if(strlen(globals.entities[i].uiComponent->text) > 0){
                                //printf("clicked changes done\n");
                            }
                            // Appearance changes when clicked
                            globals.entities[i].materialComponent->diffuseMapOpacity = globals.entities[i].materialComponent->diffuseMapOpacity * 0.5f;
                            globals.entities[i].materialComponent->diffuse.r = 0.5f;
                            globals.entities[i].materialComponent->diffuse.g = 0.0f;
                         
                            // Actions when clicked
                            if(globals.entities[i].uiComponent->onClick != NULL){
                                globals.entities[i].uiComponent->onClick();
                            }
                        }
                    
                    } else {
                         if(strlen(globals.entities[i].uiComponent->text) > 0){
                               // printf("no action,disable actions\n");
                            } 
                        globals.entities[i].uiComponent->hovered = 0;
                        globals.entities[i].uiComponent->clicked = 0;
                        globals.entities[i].materialComponent->diffuseMapOpacity = getMaterial(globals.entities[i].materialComponent->materialIndex)->diffuseMapOpacity;
                        globals.entities[i].materialComponent->diffuse.r = 0.0f;
                        globals.entities[i].materialComponent->diffuse.g= 0.0f;
                    }
            }
        }
    } 
    changeCursor(newCursor);
}

void textCursorSystem(){
           /*  printf("event btn pressed %d \n ", globals.event.button.timestamp);
            printf("event btn clicks %d \n ", globals.event.button.clicks);
            printf("event btn type %d \n ", globals.event.button.type); */

    if(globals.focusedEntityId != -1){
        if(globals.mouseLeftButtonPressed && globals.cursorEntityId != -1){
          
            // Logic to know this is not the first click
            // Logic to only run this once per click
            // Logic here to reposition cursor
            // Logic for double click, to select all text
            if(globals.mouseDoubleClick){
                printf("double clickED\n");
                selectAllText(width,height);
                // Reset state of double click
                // TODO: Why is this needed?
                globals.mouseDoubleClick = false;
            }

            
           if (globals.event.type == SDL_MOUSEBUTTONDOWN) {
            printf("Mouse button pressed\n");
            // A button was pressed
            if (globals.event.button.button == SDL_BUTTON_LEFT) {
                // Left Button Pressed
                globals.mouseLeftButtonPressed = true;
             
         
                //printf("Left button pressed\n");
            }
        }  
        if (globals.event.type == SDL_MOUSEBUTTONUP) {
            // A button was released
            globals.mouseLeftButtonPressed = false;
           


            //printf("Mouse button released\n");
        }
            // Logic for click and drag to select text
        }
        
  
        // Create cursor
        if(globals.cursorEntityId == -1){
            ClosestLetter closestLetter = getClosestLetterInText(globals.entities[globals.focusedEntityId].uiComponent, globals.mouseXpos);         
            Vector2 uiVec = convertSDLToUI(closestLetter.position.x, closestLetter.position.y,width,height);
            globals.cursorEntityId = ui_createRectangle(globals.materials[0], (vec3){uiVec.x, uiVec.y, 2.0f}, (vec3){3.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
        }
        
        // Blink cursor logic
        if((int)globals.delta_time % 2 == 0){
            globals.entities[globals.cursorEntityId].uiComponent->active = 0;
        }else {
            globals.entities[globals.cursorEntityId].uiComponent->active = 1;
        }
    }else{
        if(globals.cursorEntityId != -1){
            deleteEntity(&globals.entities[globals.cursorEntityId]);
            globals.cursorEntityId = -1;
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
void modelSystem(){
    // Look for transform needs update flag & update/recalc model matrix if needed.
     for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1) {
           if(globals.entities[i].transformComponent->modelNeedsUpdate == 1) {
                    // create transformations
                    mat4x4 model;
                    mat4x4_identity(model);

                    // set model position
                    mat4x4_translate(model, globals.entities[i].transformComponent->position[0],globals.entities[i].transformComponent->position[1],globals.entities[i].transformComponent->position[2]);

                    // set model scale
                    mat4x4_scale_aniso(model, model, globals.entities[i].transformComponent->scale[0],globals.entities[i].transformComponent->scale[1],globals.entities[i].transformComponent->scale[2]);
                
                    // rotate model on x, y, and z axes
                    // The cast on model tells the compiler that you're aware of the 
                    // const requirement and that you're promising not to modify the model matrix.
                    mat4x4 rotatedModelX, rotatedModelY, rotatedModelZ;
                    mat4x4_rotate_X(rotatedModelX, (const float (*)[4])model, globals.entities[i].transformComponent->rotation[0]);
                    mat4x4_rotate_Y(rotatedModelY, (const float (*)[4])rotatedModelX, globals.entities[i].transformComponent->rotation[1]);
                    mat4x4_rotate_Z(rotatedModelZ, (const float (*)[4])rotatedModelY, globals.entities[i].transformComponent->rotation[2]);

                    // Copy the rotated model matrix to the transform component
                    memcpy(globals.entities[i].transformComponent->transform, rotatedModelZ, sizeof(mat4x4));

                    globals.entities[i].transformComponent->modelNeedsUpdate = 0;
           }
          }}
}

void debugSystem(){
    
    // Turn off debug draw calls, we only want one frame of drawcalls saved.
    if(globals.debugDrawCalls){
        globals.debugDrawCalls = false;
    }
       
}

