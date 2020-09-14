#pragma once
#include <common.hpp>

#include <vector>


namespace rack {


/** Synchronizes plugins with the VCV Library and updates Rack itself
*/
namespace library {


struct Update {
	std::string pluginSlug;
	std::string pluginName;
	std::string version;
	std::string changelogUrl;
	float progress = 0.f;
};


extern std::string version;
extern std::string changelogUrl;
extern float progress;

void init();
bool isLoggedIn();
void logIn(const std::string& email, const std::string& password);
void logOut();
bool isUpdateAvailable();
void queryUpdates();
bool hasUpdates();
void syncUpdate(Update* update);
void syncUpdates();
bool isSyncing();
/** Updates Rack automatically or opens the browser to the URL.
Blocking. Call on a detached thread.
*/
void update();


extern std::string loginStatus;
extern std::vector<Update> updates;
extern std::string updateStatus;
extern bool restartRequested;


} // namespace library
} // namespace rack
