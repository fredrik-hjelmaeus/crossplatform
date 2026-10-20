#include "app.h"
#include "render.h"
#include "input-state.h"

#ifdef __EMSCRIPTEN__
#include "input.h"
#include "fh-sdl.h"
#include <stdio.h>
#include <emscripten.h>

typedef struct {
    AppState *app;
    RenderState *renderState;
    InputState *inputState;
} LoopContext;

static void emscriptenLoop(void *context) {
    LoopContext *loop = context;
    AppState *app = loop->app;
    RenderState *renderState = loop->renderState;
    InputState *inputState = loop->inputState;
    if(!app->running){
        printf("Closing sdl canvas!\n");
        quit(renderState, app);
        printf("Goodbye!\n");
        emscripten_cancel_main_loop();
        return;
    }
    input(inputState, renderState, app);
    update(inputState, renderState, app);
    render(renderState, app);
}

int appRun(AppState *app, RenderState *renderState, InputState *inputState)
{
    //------------------------------------------------------
    // Main loop
    // -----------------------------------------------------
    // Wasm code
    // simulate_infinite_loop keeps this context and the caller-owned states alive.
    LoopContext loop = {app, renderState, inputState};
    emscripten_set_main_loop_arg(emscriptenLoop, &loop, 0, 1);
    return 0;
}
#endif
