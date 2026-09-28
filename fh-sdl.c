#include "fh-sdl.h"
#include "globals.h"
#include <stdio.h>

void setRenderDrawColor(int r, int g, int b, int a, struct Globals *globals) {
    // Initial renderer color
    SDL_SetRenderDrawColor(globals->renderer, r, g, b, a);
}

int pollEvent(struct Globals *globals){
    return SDL_PollEvent(&globals->event);
}

int initSdlWindow(struct Globals *globals) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 0;
    }

#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

    globals->window = SDL_CreateWindow(
        "FH Engine",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (globals->window == NULL) {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

int createOpenGLContext(struct Globals *globals) {
    globals->gl_context = SDL_GL_CreateContext(globals->window);
    if (globals->gl_context == NULL) {
        fprintf(stderr, "Failed to create OpenGL context: %s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

void quit(struct Globals *globals) {
    if (globals->gl_context != NULL) SDL_GL_DeleteContext(globals->gl_context);
    if (globals->renderer != NULL) SDL_DestroyRenderer(globals->renderer);
    if (globals->window != NULL) SDL_DestroyWindow(globals->window);
    SDL_Quit();
}

void swapWindow(struct Globals *globals) { SDL_GL_SwapWindow(globals->window); }

void toggleFullscreen(struct Globals *globals) {
    Uint32 flags = SDL_GetWindowFlags(globals->window);
    SDL_SetWindowFullscreen(globals->window,
        (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void getWindowSize(struct Globals *globals, int *width, int *height) {
    SDL_GetWindowSize(globals->window, width, height);
}

void setWindowTitle(struct Globals *globals, const char *title) {
    SDL_SetWindowTitle(globals->window, title);
}

const char *getKeyName(SDL_Keycode key) { return SDL_GetKeyName(key); }
Uint32 getTicks(void) { return SDL_GetTicks(); }
void delay(Uint32 milliseconds) { SDL_Delay(milliseconds); }

void changeCursor(SDL_SystemCursor cursorType) {
    SDL_Cursor *cursor = SDL_CreateSystemCursor(cursorType);
    if (cursor != NULL) {
        SDL_SetCursor(cursor);
    } else {
        fprintf(stderr, "Failed to create cursor: %s\n", SDL_GetError());
    }
}

SDL_Keymod getModState(void) { return SDL_GetModState(); }
