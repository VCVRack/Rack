#pragma once
#include "common.hpp"


namespace rack {
namespace settings {


void save(std::string filename);
void load(std::string filename);


extern float zoom;
extern float wireOpacity;
extern float wireTension;
extern bool paramTooltip;
extern bool powerMeter;
extern bool lockModules;
extern bool checkVersion;
extern bool skipLoadOnLaunch;


} // namespace settings
} // namespace rack
