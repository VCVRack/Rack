#pragma once
#include "app/common.hpp"
#include "widgets/Widget.hpp"


namespace rack {


Widget *moduleBrowserCreate();
json_t *moduleBrowserToJson();
void moduleBrowserFromJson(json_t *rootJ);


} // namespace rack
