#include "input-state.h"

InputState inputDefaults(void)
{
    return (InputState){
        .event={0},
        .firstMouse=1,
        .mouseXpos=0.0f,
        .mouseYpos=0.0f,
        .mouseLeftButtonPressed=false, // mouse down check every frame
        .prevMouseLeftDown=false,
        .mouseDoubleClick=false,
        .mouseDragged=false,
        .mouseDragStart={-10000.0f,-10000.0f},
        .mouseDragPreviousFrame={0.0f,0.0f},
    };
}
