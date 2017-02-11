#pragma once
#include "scene.hpp"


namespace rack {


void guiInit();
void guiDestroy();
void guiRun();
void guiCursorLock();
void guiCursorUnlock();
const char *guiSaveDialog(const char *filters, const char *filename);
const char *guiOpenDialog(const char *filters, const char *filename);

extern NVGcontext *gVg;

} // namespace rack
