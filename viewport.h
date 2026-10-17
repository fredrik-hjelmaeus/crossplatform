#ifndef VIEWPORT_H
#define VIEWPORT_H

typedef struct AppState AppState;

#include "types.h"



void setViewport(View view);
void setViewportAndClear(AppState *app, View view);
void setViewportWithScissorAndClear(AppState *app, View view);

void updateUIonViewportChange(AppState *app, 
  float prevWidth, 
  float prevHeight,
  float ui_percentageWidth,
  float main_percentageWidth,
  float ui_percentageHeight
);
  
void recalculateViewports(int w, int h, AppState *app);

#endif // VIEWPORT_H
