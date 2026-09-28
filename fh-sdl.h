#ifndef FH_SDL_H
#define FH_SDL_H
#include <SDL2/SDL.h>
#include "globals.h"

void setRenderDrawColor(int r, int g, int b, int a, struct Globals *globals);
int pollEvent(struct Globals *globals);
int initSdlWindow(struct Globals *globals);
int createOpenGLContext(struct Globals *globals);
void quit(struct Globals *globals);
void swapWindow(struct Globals *globals);
void toggleFullscreen(struct Globals *globals);
void getWindowSize(struct Globals *globals, int *width, int *height);
void setWindowTitle(struct Globals *globals, const char *title);
const char *getKeyName(SDL_Keycode key);
Uint32 getTicks(void);
void delay(Uint32 milliseconds);
void changeCursor(SDL_SystemCursor cursorType);
SDL_Keymod getModState(void);

#endif // FH_SDL_H
