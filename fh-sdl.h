#ifndef FH_SDL_H
#define FH_SDL_H

typedef struct InputState InputState;

typedef struct RenderState RenderState;

typedef struct AppState AppState;
#include <SDL2/SDL.h>

void setRenderDrawColor(RenderState *renderState, int r, int g, int b, int a);
int pollEvent(InputState *inputState);
int initSdlWindow(AppState *app);
int createOpenGLContext(RenderState *renderState, AppState *app);
void quit(RenderState *renderState, AppState *app);
void swapWindow(AppState *app);
void toggleFullscreen(AppState *app);
void getWindowSize(AppState *app, int *width, int *height);
void setWindowTitle(AppState *app, const char *title);
const char *getKeyName(SDL_Keycode key);
Uint32 getTicks(void);
void delay(Uint32 milliseconds);
void changeCursor(SDL_SystemCursor cursorType);
SDL_Keymod getModState(void);

#endif // FH_SDL_H
