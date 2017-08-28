#pragma once
#include "app.hpp"


namespace rack {


void guiInit();
void guiDestroy();
void guiRun();
bool guiIsKeyPressed(int key);
void guiCursorLock();
void guiCursorUnlock();

extern NVGcontext *gVg;
extern std::shared_ptr<Font> gGuiFont;
extern float gPixelRatio;

} // namespace rack
