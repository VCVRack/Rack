#pragma once

#include <string>


namespace rack {


extern bool gSkipAutosaveOnLaunch;
extern bool b_touchkeyboard_enable;

void settingsSave(std::string filename);
void settingsLoad(std::string filename, bool bWindowSizeOnly);


} // namespace rack
