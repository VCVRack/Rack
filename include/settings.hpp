#pragma once

#include <string>


namespace rack {


void settingsSave(std::string filename);
void settingsLoad(std::string filename);


extern bool skipAutosaveOnLaunch;


} // namespace rack
