#pragma once
#include "app/common.hpp"


namespace rack {


void moduleBrowserCreate();
json_t *moduleBrowserToJson();
void moduleBrowserFromJson(json_t *rootJ);


} // namespace rack
