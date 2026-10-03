#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "types.h"

struct Globals;

void setViewport(View view);
void setViewportAndClear(View view);
void setViewportWithScissorAndClear(View view);

void updateUIonViewportChange(
  float prevWidth, 
  float prevHeight,
  float ui_percentageWidth,
  float main_percentageWidth,
  float ui_percentageHeight);
  
void recalculateViewports(int w, int h, struct Globals *globals);

#endif // VIEWPORT_H
