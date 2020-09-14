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


void init();
void destroy();

bool isAppUpdateAvailable();

bool isLoggedIn();
void logIn(const std::string& email, const std::string& password);
void logOut();
void queryUpdates();
bool hasUpdates();
void syncUpdate(Update* update);
void syncUpdates();
bool isSyncing();


extern std::string appVersion;
extern std::string appDownloadUrl;
extern std::string appChangelogUrl;

extern std::string loginStatus;
extern std::vector<Update> updates;
extern std::string updateStatus;
extern bool restartRequested;


} // namespace library
} // namespace rack
