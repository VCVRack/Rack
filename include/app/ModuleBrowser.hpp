#pragma once
#include "app/common.hpp"
#include "widget/Widget.hpp"


namespace rack {
namespace app {


widget::Widget *moduleBrowserCreate();
json_t *moduleBrowserToJson();
void moduleBrowserFromJson(json_t *rootJ);


} // namespace app
} // namespace rack
