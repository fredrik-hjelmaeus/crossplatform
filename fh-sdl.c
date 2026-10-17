#include "fh-sdl.h"
#include "app.h"
#include <stdio.h>

void setRenderDrawColor(int r, int g, int b, int a, AppState *app) {
    // Initial renderer color
    SDL_SetRenderDrawColor(app->renderer, r, g, b, a);
}

int pollEvent(AppState *app){
    return SDL_PollEvent(&app->event);
}

int initSdlWindow(AppState *app) {
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

    app->window = SDL_CreateWindow(
        "FH Engine",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width,
        height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (app->window == NULL) {
        fprintf(stderr, "%s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

int createOpenGLContext(AppState *app) {
    app->gl_context = SDL_GL_CreateContext(app->window);
    if (app->gl_context == NULL) {
        fprintf(stderr, "Failed to create OpenGL context: %s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

void quit(AppState *app) {
    if (app->gl_context != NULL) SDL_GL_DeleteContext(app->gl_context);
    if (app->renderer != NULL) SDL_DestroyRenderer(app->renderer);
    if (app->window != NULL) SDL_DestroyWindow(app->window);
    SDL_Quit();
}

void swapWindow(AppState *app) { SDL_GL_SwapWindow(app->window); }

void toggleFullscreen(AppState *app) {
    Uint32 flags = SDL_GetWindowFlags(app->window);
    SDL_SetWindowFullscreen(app->window,
        (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void getWindowSize(AppState *app, int *width, int *height) {
    SDL_GetWindowSize(app->window, width, height);
}

void setWindowTitle(AppState *app, const char *title) {
    SDL_SetWindowTitle(app->window, title);
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
