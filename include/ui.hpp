#pragma once
#include "ui/common.hpp"
#include "color.hpp"
#include "window.hpp"


namespace rack {


void uiInit();
void uiDestroy();
void uiSetTheme(NVGcolor bg, NVGcolor fg);


} // namespace rack
