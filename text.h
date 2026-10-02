#ifndef TEXT_H
#define TEXT_H

#include "types.h"

ClosestLetter getCharacterByIndex(int index);
ClosestLetter getClosestLetterInText(UIComponent* uiComponent,BoundingBoxComponent* bbComponent, float mouseX);
void selectAllText(int width, int height);
void handleDeleteButton(int width,int height);
ClosestLetter findCharacterUnderCursor(int width,int height);
void removeCharacter(int index);
bool isCapsLock(void);
bool isLeftShiftPressed(void);
char toUpperCase(char c);
char toLowerCase(char c);
char specialLeftShiftHandling(char c);
void deleteTextRange(unsigned int startIndex, unsigned int endIndex);
void addIndexToCursorTextSelection(unsigned int index);

/**
 * Initialize the font
 * Load texture,setup mesh,setup material
 * After this the projection matrix for the font need to be set using setFontProjection.
 * And then you can render text using renderText.
 */
void initFont(void);


#endif
