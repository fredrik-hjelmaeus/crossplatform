#ifndef VIEWPORT_H
#define VIEWPORT_H

void updateUIonViewportChange(
  float prevWidth, 
  float prevHeight,
  float ui_percentageWidth,
  float main_percentageWidth,
  float ui_percentageHeight);
  
void recalculateViewports(int w, int h, struct Globals *globals);

#endif // VIEWPORT_H