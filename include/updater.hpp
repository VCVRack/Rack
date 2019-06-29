#pragma once
#include <common.hpp>


namespace rack {


/** Automatically updates the application.
*/
namespace updater {


extern std::string version;
extern std::string changelogUrl;
extern float progress;

void init();
bool isUpdateAvailable();
/** Updates Rack automatically or opens the browser to the URL.
Blocking. Call on a detached thread.
*/
void update();


} // namespace updater
} // namespace rack
