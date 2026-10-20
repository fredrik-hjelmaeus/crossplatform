#include "app.h"
#include "render.h"
#include "input-state.h"

int main(int argc, char **argv) {

    AppState appState = appDefaults();
    RenderState renderState = renderDefaults();
    InputState inputState = inputDefaults();

    appInit(&appState, &renderState);
    
    return appRun(&appState, &renderState, &inputState);

}
