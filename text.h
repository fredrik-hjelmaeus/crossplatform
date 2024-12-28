#ifndef TEXT_H
#define TEXT_H

#include "types.h"

ClosestLetter getCharacterByIndex(int index);
ClosestLetter getClosestLetterInText(UIComponent* uiComponent, float mouseX);
void selectAllText(int width, int height);
void handleDeleteButton(int width,int height);
ClosestLetter findCharacterUnderCursor(int width,int height);
void removeCharacter(int index);
bool isCapsLock();
bool isLeftShiftPressed();
char toUpperCase(char c);
char toLowerCase(char c);
char specialLeftShiftHandling(char c);
void deleteTextRange(unsigned int startIndex, unsigned int endIndex);
void addIndexToCursorTextSelection(unsigned int index);


#endif
