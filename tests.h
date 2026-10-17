#ifndef TESTS_H
#define TESTS_H

typedef struct InputState InputState;

typedef struct AppState AppState;

void tests_ui_coordinate_conversion(void);
void tests_binary_conversion(void);
void tests_events(InputState *inputState, AppState *app);

#endif // End of the TESTS_H definition
