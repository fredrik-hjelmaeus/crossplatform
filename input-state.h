#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "linmath.h"

// Device events and mouse tracking, independent of UI focus and selection.
typedef struct InputState {
    SDL_Event event;
    int firstMouse;
    float mouseXpos;
    float mouseYpos;
    bool mouseLeftButtonPressed;
    bool prevMouseLeftDown;
    bool mouseDragged;
    vec2 mouseDragStart;
    vec2 mouseDragPreviousFrame;
    bool mouseDoubleClick;
} InputState;

InputState inputDefaults(void);

#endif // INPUT_STATE_H
