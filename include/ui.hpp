#pragma once
#include "ui/common.hpp"
#include "color.hpp"


namespace rack {
namespace ui {


void init();
void destroy();
void setTheme(NVGcolor bg, NVGcolor fg);


} // namespace ui
} // namespace rack
