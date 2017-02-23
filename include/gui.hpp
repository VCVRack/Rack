#pragma once
#include "app.hpp"


namespace rack {


void guiInit();
void guiDestroy();
void guiRun();
bool guiIsKeyPressed(int key);
void guiCursorLock();
void guiCursorUnlock();
const char *guiSaveDialog(const char *filters, const char *filename);
const char *guiOpenDialog(const char *filters, const char *filename);

extern NVGcontext *gVg;

} // namespace rack
