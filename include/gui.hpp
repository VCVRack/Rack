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

// TODO This should probably go in another file, like resources.hpp?
void drawSVG(NVGcontext *vg, NSVGimage *svg);

} // namespace rack
