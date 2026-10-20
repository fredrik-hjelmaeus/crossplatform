#include "app.h"
#include "render.h"
#include "input-state.h"

#ifndef __EMSCRIPTEN__
#include "input.h"
#include "fh-sdl.h"
#include <stdio.h>
#include "ecs-systems.h"

int appRun(AppState *app, RenderState *renderState, InputState *inputState)
{
    //------------------------------------------------------
    // Main loop
    // -----------------------------------------------------
    // native code
    while(app->running) {
        input(inputState, renderState, app);
        update(inputState, renderState, app);
        render(renderState, app);
        debugSystem(renderState);
    }

    // -----------------------------------------------------
    // Quit
    // -----------------------------------------------------
    quit(renderState, app);
    printf("Non emscripten shutdown complete!\n");
    return 0;
}
#endif
