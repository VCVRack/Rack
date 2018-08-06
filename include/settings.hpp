#pragma once

#include <string>


namespace rack {


extern bool gSkipAutosaveOnLaunch;

void settingsSave(std::string filename);
void settingsLoad(std::string filename, bool bWindowSizeOnly);


} // namespace rack
