#include "text.h"
#include "types.h"
#include "utils.h"
#include "globals.h"
#include "api.h"

ClosestLetter getCharacterByIndex(int index){

    const char* text = globals.entities[globals.focusedEntityId].uiComponent->text;
    if(index > strlen(text)){
        printf("unhandled path");
        exit(1);
    }
    float x = (float)globals.entities[globals.focusedEntityId].uiComponent->boundingBox.x; 
    float y = (float)globals.entities[globals.focusedEntityId].uiComponent->boundingBox.y 
    + ((float)globals.entities[globals.focusedEntityId].uiComponent->boundingBox.height / 2);
    float scale = globals.charScale;
    float xpos = 0.0f;
    float ypos = 0.0f;
    float lastShift = 0.0f;
    ClosestLetter closestLetter;
    closestLetter.characterIndex = 0;
    closestLetter.position = (Vector2){0.0f, 0.0f};

    for (unsigned char c = 0; c <= index; c++) {
        Character ch = globals.characters[text[c]];

        // Calculate the position of the current character
        xpos = x + (float)ch.Bearing[0] * scale;
        ypos = y - ((float)ch.Size[1] - (float)ch.Bearing[1]) * scale;

        float w = (float)ch.Size[0] * scale;
        float h = (float)ch.Size[1] * scale;    
        
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        lastShift = (float)(ch.Advance >> 6) * scale;
        x += lastShift; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        closestLetter.characterIndex = c;
        closestLetter.charWidth = (float)ch.Bearing[0] * scale + lastShift; 
        closestLetter.position.x = xpos;
        closestLetter.position.y = ypos;
    }

    return closestLetter;
}

void deleteTextRange(unsigned int startIndex, unsigned int endIndex){
    char* text = globals.entities[globals.focusedEntityId].uiComponent->text;
    char* newText = (char*)arena_Alloc(&globals.assetArena, 99 * sizeof(char));
    
    int length = strlen(text);
    if(length == 0){
        return;
    }
    if(startIndex > length || endIndex > length){
        return;
    }
    if(startIndex > endIndex){
        return;
    }
    int j = 0;
    for(int i = 0; i < length; i++){
       if(i >= startIndex && i < endIndex){
         continue;
       }
       newText[j] = text[i];
       j++;
    }
    newText[j] = '\0';
    globals.entities[globals.focusedEntityId].uiComponent->text = newText;
    globals.cursorSelectionActive = false;
    globals.cursorTextSelection[0] = 0;
    globals.cursorTextSelection[1] = 0;
}


void addIndexToCursorTextSelection(unsigned int index){
    // On the first call, initialize both to the index
    if (globals.cursorTextSelection[0] == 0 && globals.cursorTextSelection[1] == 0) {
        globals.cursorTextSelection[0] = index;
        globals.cursorTextSelection[1] = index;
        return;
    }

    if (index < globals.cursorTextSelection[0]) {
        globals.cursorTextSelection[0] = index;
    } else if (index > globals.cursorTextSelection[1]) {
        globals.cursorTextSelection[1] = index;
    }
}
/**
 * @brief Get the closest letter position in the text to the given mouse X position.
 * 
 * @param uiComponent The UI component containing the text.
 * @param mouseX The X position of the mouse in SDL coordinates.
 * @return Vector2 The position of the closest letter.
 */
ClosestLetter getClosestLetterInText(UIComponent* uiComponent, float mouseX){
    char* text = uiComponent->text;
    float x = (float)uiComponent->boundingBox.x; 
    float y = (float)uiComponent->boundingBox.y + ((float)uiComponent->boundingBox.height / 2);   //(float)globals.characters[0].Size[1]; 
    float scale = globals.charScale; // Character size, also set when renderText is called (they should be in sync)
    float xpos = 0.0f;
    float ypos = 0.0f;
    float bestCharX = (float)uiComponent->boundingBox.x + (float)uiComponent->boundingBox.width;
    float lastShift = 0.0f;
    ClosestLetter closestLetter;
    closestLetter.characterIndex = 0;
    closestLetter.position = (Vector2){x, 0.0f};
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
            closestLetter.charWidth = w + (float)ch.Bearing[0] * scale;//lastShift; // (float)ch.Bearing[0] * scale; //+ lastShift;
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

   globals.cursorSelectionActive = true;

    // Calculate width of the input field
    float xMin = globals.entities[globals.focusedEntityId].uiComponent->boundingBox.x;
    float xMax = xMin + globals.entities[globals.focusedEntityId].uiComponent->boundingBox.width;

    // Find the first and last letter in the text using max and min x values of the input field
    ClosestLetter firstLetter = getClosestLetterInText(globals.entities[globals.focusedEntityId].uiComponent, xMin );
    ClosestLetter lastLetter =  getClosestLetterInText(globals.entities[globals.focusedEntityId].uiComponent, xMax );

    // Convert to UI coordinates
    SDLVector2 firstLetterSDLpos;
    firstLetterSDLpos.x = firstLetter.position.x;
    firstLetterSDLpos.y = firstLetter.position.y;
    SDLVector2 lastLetterSDLpos;
    lastLetterSDLpos.x = lastLetter.position.x;
    lastLetterSDLpos.y = lastLetter.position.y;
    UIVector2 firstLetterUIpos = convertSDLToUI(firstLetterSDLpos,width,height);
    UIVector2 lastLetterUIpos = convertSDLToUI(lastLetterSDLpos,width,height);

    // Calculate position of the rectangle
    float rectangleStartPos = firstLetterUIpos.x - (lastLetterUIpos.x * 0.5);

    // Set cursor to be a new width & position
    globals.entities[globals.cursorEntityId].transformComponent->position[0] = rectangleStartPos;
    globals.entities[globals.cursorEntityId].transformComponent->scale[0] = lastLetterUIpos.x;
    globals.entities[globals.cursorEntityId].transformComponent->modelNeedsUpdate = 1;
    
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
    ASSERT(globals.focusedEntityId != -1, "No focused entity");
    ASSERT(globals.cursorEntityId != -1, "No cursor entity");

    UIVector2 uiVec;
    uiVec.x = globals.entities[globals.cursorEntityId].transformComponent->position[0];
    uiVec.y = globals.entities[globals.cursorEntityId].transformComponent->position[1];
    SDLVector2 sdlVec = convertUIToSDL(uiVec, width, height);
  
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
