#pragma once
#include "common.hpp"


namespace rack {
namespace settings {


extern bool gSkipAutosaveOnLaunch;

void save(std::string filename);
void load(std::string filename);


} // namespace settings
} // namespace rack
