#ifndef TEXT_H
#define TEXT_H

typedef struct AppState AppState;

#include "types.h"

ClosestLetter getCharacterByIndex(AppState *app, int index);
ClosestLetter getClosestLetterInText(AppState *app, UIComponent* uiComponent,BoundingBoxComponent* bbComponent, float mouseX);
void selectAllText(AppState *app, int width, int height);
void handleDeleteButton(AppState *app, int width,int height);
ClosestLetter findCharacterUnderCursor(AppState *app, int width,int height);
void removeCharacter(AppState *app, int index);
bool isCapsLock(void);
bool isLeftShiftPressed(void);
char toUpperCase(char c);
char toLowerCase(char c);
char specialLeftShiftHandling(char c);
void deleteTextRange(AppState *app, unsigned int startIndex, unsigned int endIndex);
void addIndexToCursorTextSelection(AppState *app, unsigned int index);

/**
 * Initialize the font
 * Load texture,setup mesh,setup material
 * After this the projection matrix for the font need to be set using setFontProjection.
 * And then you can render text using renderText.
 */
void initFont(AppState *app);


#endif
