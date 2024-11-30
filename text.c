#include "text.h"
#include "types.h"
#include "utils.h"
#include "globals.h"

/**
 * @brief Get the closest letter position in the text to the given mouse X position.
 * 
 * @param uiComponent The UI component containing the text.
 * @param mouseX The X position of the mouse.
 * @return Vector2 The position of the closest letter.
 */
ClosestLetter getClosestLetterInText(UIComponent* uiComponent, float mouseX){
    const char* text = uiComponent->text;
    float x = (float)uiComponent->boundingBox.x; 
    float y = (float)uiComponent->boundingBox.y + ((float)uiComponent->boundingBox.height / 2);   //(float)globals.characters[0].Size[1]; 
    float scale = globals.charScale; // Character size, also set when renderText is called (they should be in sync)
    float xpos = 0.0f;
    float ypos = 0.0f;
    float bestCharX = (float)uiComponent->boundingBox.x + (float)uiComponent->boundingBox.width;
    float lastShift = 0.0f;
    ClosestLetter closestLetter;
    closestLetter.characterIndex = 0;
    closestLetter.position = (Vector2){0.0f, 0.0f};
    if(strlen(text) == 0){
        return closestLetter;
    }
        
    for (unsigned char c = 0; c < strlen(text); c++) {
        Character ch = globals.characters[text[c]];

        // Calculate the position of the current character
        xpos = x + (float)ch.Bearing[0] * scale;
        ypos = y - ((float)ch.Size[1] - (float)ch.Bearing[1]) * scale;

        float w = (float)ch.Size[0] * scale;
        float h = (float)ch.Size[1] * scale;    
       
        // Check if the mouse is closer to the current character than the previous closest character
        float testxpos = absValue(mouseX - xpos);
        float testbestCharX = absValue(mouseX - bestCharX);
           
        if(testxpos > bestCharX){
            // Previous character was the closest,so we remove the last character width from the xpos.
            closestLetter.position.x = xpos - (float)ch.Bearing[0] * scale - lastShift;
            closestLetter.position.y = ypos;
            closestLetter.characterIndex = c - 1; 
            closestLetter.charWidth = (float)ch.Bearing[0] * scale + lastShift;
            ASSERT(closestLetter.position.x >= 0, "closestLetter.position.x is negative");
            ASSERT(closestLetter.position.y >= 0, "closestLetter.position.y is negative");
            ASSERT(closestLetter.characterIndex >= 0, "closestLetter.characterIndex is negative");
            ASSERT(closestLetter.charWidth >= 0, "closestLetter.charWidth is negative");
            return closestLetter;
        }
        bestCharX = testxpos;
   
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        lastShift = (float)(ch.Advance >> 6) * scale;
        x += lastShift; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        closestLetter.characterIndex++;
        closestLetter.charWidth = (float)ch.Bearing[0] * scale + lastShift; 
   } 
   
   // Determine which side of the character is closest to the mouse between last character and penultimate character.
   float lastCharPos = absValue(mouseX - (xpos + lastShift));
   float prevCharPos = absValue(mouseX - xpos);
   float result = prevCharPos > lastCharPos ? (xpos+lastShift) : xpos; 

   closestLetter.position.x = result;
   closestLetter.position.y = ypos;
   closestLetter.characterIndex = prevCharPos > lastCharPos ? closestLetter.characterIndex : (closestLetter.characterIndex - 1);
            
   ASSERT(closestLetter.position.x >= 0, "closestLetter.position.x is negative(e)");
   ASSERT(closestLetter.position.y >= 0, "closestLetter.position.y is negative(e)");
   ASSERT(closestLetter.characterIndex >= 0, "closestLetter.characterIndex is negative(e)");
   ASSERT(closestLetter.charWidth >= 0, "closestLetter.charWidth is negative(e)");

   return closestLetter;
}

// Select all text fn
void selectAllText(int width, int height){
    ASSERT(globals.focusedEntityId != -1, "No focused entity");
    ASSERT(globals.cursorEntityId != -1, "No cursor entity");
    int length = strlen(globals.entities[globals.focusedEntityId].uiComponent->text);
    if(length == 0){
        return;
    }
    // Calculate width of the text
    float xMin = globals.entities[globals.focusedEntityId].uiComponent->boundingBox.x;
    float xMax = xMin + globals.entities[globals.focusedEntityId].uiComponent->boundingBox.width;
    Vector2 uiMax = convertSDLToUI(xMax, 0.0f,width,height);
    Vector2 uiMin = convertSDLToUI(xMin, 0.0f,width,height);
    float endOfInputField = uiMax.x-uiMin.x;
 //   printf("uiMax x %f\n", uiMax.x);
 //   printf("uiMin x %f\n", uiMin.x);
   //  ClosestLetter closestLetter = getClosestLetterInText(globals.entities[globals.focusedEntityId].uiComponent, globals.mouseXpos);         
     //       Vector2 uiVec = convertSDLToUI(closestLetter.position.x, closestLetter.position.y);


    ClosestLetter firstLetter = getClosestLetterInText(globals.entities[globals.focusedEntityId].uiComponent,globals.mouseXpos );
    Vector2 uiVec = convertSDLToUI(firstLetter.position.x, firstLetter.position.y,width,height);
    ui_createRectangle(globals.materials[0], (vec3){uiVec.x, uiVec.y, 2.0f}, (vec3){35.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
   
    printf("first letter x %d\n", uiVec.x);
    printf("last letter x %d\n", uiVec.x);
}

/**
 * @brief Delete the character to the left of the cursor.
 * Triggers on backspace key press.
 */
void handleDeleteButton(int width,int height){
    if(strlen(globals.entities[globals.focusedEntityId].uiComponent->text) == 0){
        return;
    }
    ClosestLetter closestLetter = findCharacterUnderCursor(width,height);
                 
    // Remove the letter to the left of the cursor
    removeCharacter(closestLetter.characterIndex);
}

ClosestLetter findCharacterUnderCursor(int width,int height){
    // Find out where the cursor is in the text
    // Find the closest letter to the cursor (getClosestLetterInText) probably need to also return the index of the letter in the text and its char-width.
    ASSERT(globals.focusedEntityId != -1, "No focused entity");
    ASSERT(globals.cursorEntityId != -1, "No cursor entity");
 
    Vector2 sdlVec = convertUIToSDL(globals.entities[globals.cursorEntityId].transformComponent->position[0], 
    globals.entities[globals.cursorEntityId].transformComponent->position[1],
    width,height);
  
    ASSERT(sdlVec.x >= 0, "Sdl cursor x position is negative");
    ASSERT(sdlVec.y >= 0, "Sdl cursor y position is negative");
    return getClosestLetterInText(
                    globals.entities[globals.focusedEntityId].uiComponent,
                    sdlVec.x
        );
}

/**
 * @brief Removes the character on the index. Example: "TextIn|put" -> removeCharacter(6) "TextInut"
 */
void removeCharacter(int index){
    if(index < 0){
        return;
    }
    
    char* originalText = globals.entities[globals.focusedEntityId].uiComponent->text;
    int originalLength = strlen(originalText);
    
    // Allocate memory for the new string (original length - 1 character + null terminator)
    char* textCopy = (char*)malloc(originalLength * sizeof(char));
      if (textCopy == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    int j = 0;
    for(int i = 0; i < originalLength; i++){
        if(i == index){
            continue;
        }else{
            textCopy[j] = originalText[i];
            j++;
        }
    }
    textCopy[j] = '\0'; // null terminate
    
    globals.entities[globals.focusedEntityId].uiComponent->text = textCopy; // assign
}

// TODO: temp solution, do not handle if caps lock is already active on program run.
bool isCapsLock() {
    SDL_Keymod modState = SDL_GetModState();
    if (modState & KMOD_CAPS) {
        //printf("Caps Lock is activated\n");
        return true;
    } else {
        //printf("Caps Lock is not activated\n");
        return false;
    }
}

// TODO: This is a temp solution. We should handle this using
// event->key.keysym.sym == SDLK_LSHIFT instead.
bool isLeftShiftPressed(){
    SDL_Keymod modState = SDL_GetModState();
    if (modState & KMOD_LSHIFT) {
        //printf("Left Shift is activated\n");
        return true;
    } else {
        //printf("Left Shift is not activated\n");
        return false;
    }
}

char toUpperCase(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 32;
    }
    return c; // Return the character unchanged if it's not a lowercase letter
}

char toLowerCase(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c; // Return the character unchanged if it's not an uppercase letter
}

char specialLeftShiftHandling(char c){
    if(c == '1'){
        return '!';
    }
    if(c == '2'){
        return '"';
    }
    if(c == '3'){
        return '#';
    }
    if(c == '4'){
        return '$';
    }
    if(c == '5'){
        return '%';
    }
    if(c == '6'){
        return '^';
    }
    if(c == '7'){
        return '&';
    }
    if(c == '8'){
        return '*';
    }
    if(c == '9'){
        return '(';
    }
    if(c == '0'){
        return ')';
    }
    return c;
}