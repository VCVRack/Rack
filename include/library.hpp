#pragma once
#include <common.hpp>

#include <map>


namespace rack {
/** Synchronizes plugins with the VCV Library and handles VCV accounts with the vcvrack.com API */
namespace library {


struct UpdateInfo {
	std::string name;
	std::string version;
	std::string changelogUrl;
	bool downloaded = false;
};


void init();
void destroy();

void checkAppUpdate();
bool isAppUpdateAvailable();

bool isLoggedIn();
void logIn(const std::string& email, const std::string& password);
void logOut();
void checkUpdates();
bool hasUpdates();
void syncUpdate(const std::string& slug);
void syncUpdates();


extern std::string appVersion;
extern std::string appDownloadUrl;
extern std::string appChangelogUrl;

extern std::string loginStatus;
// plugin slug -> UpdateInfo
extern std::map<std::string, UpdateInfo> updateInfos;
extern std::string updateStatus;
extern std::string updateSlug;
extern float updateProgress;
/** Whether plugins are currently downloading. */
extern bool isSyncing;
/** Whether the UI should ask the user to restart after updating plugins. */
extern bool restartRequested;


} // namespace library
} // namespace rack
