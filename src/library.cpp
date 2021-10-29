#include <thread>
#include <mutex>
#include <condition_variable>

#include <library.hpp>
#include <settings.hpp>
#include <app/common.hpp>
#include <network.hpp>
#include <system.hpp>
#include <context.hpp>
#include <window/Window.hpp>
#include <asset.hpp>
#include <settings.hpp>
#include <plugin.hpp>


namespace rack {
namespace library {


static std::mutex appUpdateMutex;
static std::mutex updateMutex;


void init() {
	if (settings::autoCheckUpdates && !settings::devMode) {
		std::thread t([&]() {
			system::setThreadName("Library");
			// checkAppUpdate();
			checkUpdates();
		});
		t.detach();
	}
}


void destroy() {
	// Wait until all library threads are finished
	{
		std::lock_guard<std::mutex> lock(appUpdateMutex);
	}
	{
		std::lock_guard<std::mutex> lock(updateMutex);
	}
}


void checkAppUpdate() {
	if (!appUpdateMutex.try_lock())
		return;
	DEFER({appUpdateMutex.unlock();});

	std::string versionUrl = API_URL + "/version";
	json_t* reqJ = json_object();
	json_object_set(reqJ, "edition", json_string(APP_EDITION.c_str()));
	DEFER({json_decref(reqJ);});

	json_t* resJ = network::requestJson(network::METHOD_GET, versionUrl, reqJ);
	if (!resJ) {
		WARN("Request for version failed");
		return;
	}
	DEFER({json_decref(resJ);});

	json_t* versionJ = json_object_get(resJ, "version");
	if (versionJ)
		appVersion = json_string_value(versionJ);

	json_t* changelogUrlJ = json_object_get(resJ, "changelogUrl");
	if (changelogUrlJ)
		appChangelogUrl = json_string_value(changelogUrlJ);

	json_t* downloadUrlsJ = json_object_get(resJ, "downloadUrls");
	if (downloadUrlsJ) {
		json_t* downloadUrlJ = json_object_get(downloadUrlsJ, APP_ARCH.c_str());
		if (downloadUrlJ)
			appDownloadUrl = json_string_value(downloadUrlJ);
	}
}


bool isAppUpdateAvailable() {
	return (appVersion != "") && (appVersion != APP_VERSION);
}


bool isLoggedIn() {
	return settings::token != "";
}


void logIn(std::string email, std::string password) {
	if (!updateMutex.try_lock())
		return;
	DEFER({updateMutex.unlock();});

	loginStatus = "Logging in...";
	json_t* reqJ = json_object();
	json_object_set(reqJ, "email", json_string(email.c_str()));
	json_object_set(reqJ, "password", json_string(password.c_str()));
	std::string url = API_URL + "/token";
	json_t* resJ = network::requestJson(network::METHOD_POST, url, reqJ);
	json_decref(reqJ);

	if (!resJ) {
		loginStatus = "No response from server";
		return;
	}
	DEFER({json_decref(resJ);});

	json_t* errorJ = json_object_get(resJ, "error");
	if (errorJ) {
		const char* errorStr = json_string_value(errorJ);
		loginStatus = errorStr;
		return;
	}

	json_t* tokenJ = json_object_get(resJ, "token");
	if (!tokenJ) {
		loginStatus = "No token in response";
		return;
	}

	const char* tokenStr = json_string_value(tokenJ);
	settings::token = tokenStr;
	loginStatus = "";
	refreshRequested = true;
}


void logOut() {
	settings::token = "";
	updateInfos.clear();
}


static network::CookieMap getTokenCookies() {
	network::CookieMap cookies;
	cookies["token"] = settings::token;
	return cookies;
}

void checkUpdates() {
	if (!updateMutex.try_lock())
		return;
	DEFER({updateMutex.unlock();});

	if (settings::token.empty())
		return;

	// Refuse to check for updates while updating plugins
	if (isSyncing)
		return;

	updateStatus = "Querying for updates...";

	// Check user token
	std::string userUrl = API_URL + "/user";
	json_t* userResJ = network::requestJson(network::METHOD_GET, userUrl, NULL, getTokenCookies());
	if (!userResJ) {
		DEBUG("User failed");
		return;
	}
	DEFER({json_decref(userResJ);});

	// Get library manifests
	std::string manifestsUrl = API_URL + "/library/manifests";
	json_t* manifestsReq = json_object();
	json_object_set(manifestsReq, "version", json_string(APP_VERSION_MAJOR.c_str()));
	json_t* manifestsResJ = network::requestJson(network::METHOD_GET, manifestsUrl, manifestsReq);
	json_decref(manifestsReq);
	if (!manifestsResJ) {
		WARN("Request for library manifests failed");
		updateStatus = "Could not query plugin manifests";
		return;
	}
	DEFER({json_decref(manifestsResJ);});

	// Get user's plugins list
	std::string pluginsUrl = API_URL + "/plugins";
	json_t* pluginsResJ = network::requestJson(network::METHOD_GET, pluginsUrl, NULL, getTokenCookies());
	if (!pluginsResJ) {
		WARN("Request for user's plugins failed");
		updateStatus = "Could not query user's plugins";
		return;
	}
	DEFER({json_decref(pluginsResJ);});

	json_t* errorJ = json_object_get(pluginsResJ, "error");
	if (errorJ) {
		WARN("Request for user's plugins returned an error: %s", json_string_value(errorJ));
		updateStatus = "Could not query plugins";
		return;
	}

	json_t* manifestsJ = json_object_get(manifestsResJ, "manifests");
	json_t* pluginsJ = json_object_get(pluginsResJ, "plugins");

	size_t pluginIndex;
	json_t* pluginJ;
	json_array_foreach(pluginsJ, pluginIndex, pluginJ) {
		// Get plugin manifest
		std::string slug = json_string_value(pluginJ);
		json_t* manifestJ = json_object_get(manifestsJ, slug.c_str());
		if (!manifestJ) {
			WARN("VCV account has plugin %s but no manifest was found", slug.c_str());
			continue;
		}

		// Don't replace existing UpdateInfo, even if version is newer.
		// This keeps things sane and ensures that only one version of each plugin is downloaded to `plugins/` at a time.
		auto it = updateInfos.find(slug);
		if (it != updateInfos.end()) {
			continue;
		}
		UpdateInfo update;

		// Get plugin name
		json_t* nameJ = json_object_get(manifestJ, "name");
		if (nameJ)
			update.name = json_string_value(nameJ);

		// Get version
		json_t* versionJ = json_object_get(manifestJ, "version");
		if (!versionJ) {
			// WARN("Plugin %s has no version in manifest", slug.c_str());
			continue;
		}
		update.version = json_string_value(versionJ);
		// Reject plugins with ABI mismatch
		if (!string::startsWith(update.version, APP_VERSION_MAJOR + ".")) {
			continue;
		}

		// Check if update is needed
		plugin::Plugin* p = plugin::getPlugin(slug);
		if (p && p->version == update.version)
			continue;

		// Require that plugin is available
		json_t* availableJ = json_object_get(manifestJ, "available");
		if (!json_boolean_value(availableJ))
			continue;

		// Get changelog URL
		json_t* changelogUrlJ = json_object_get(manifestJ, "changelogUrl");
		if (changelogUrlJ)
			update.changelogUrl = json_string_value(changelogUrlJ);

		// Add update to updates map
		updateInfos[slug] = update;
	}

	// Get module whitelist
	{
		std::string whitelistUrl = API_URL + "/modules";
		json_t* whitelistResJ = network::requestJson(network::METHOD_GET, whitelistUrl, NULL, getTokenCookies());
		if (!whitelistResJ) {
			WARN("Request for module whitelist failed");
			updateStatus = "Could not query user's modules";
			return;
		}
		DEFER({json_decref(whitelistResJ);});

		// Clone plugin slugs from settings to temporary whitelist.
		// This makes plugins entirely hidden if removed.
		std::map<std::string, std::set<std::string>> moduleWhitelist;
		for (const auto& pluginPair : settings::moduleWhitelist) {
			// Create an empty set
			moduleWhitelist[pluginPair.first];
		}

		// Iterate plugins
		json_t* pluginsJ = json_object_get(whitelistResJ, "plugins");
		const char* pluginSlug;
		json_t* modulesJ;
		json_object_foreach(pluginsJ, pluginSlug, modulesJ) {
			// Iterate modules in plugin
			size_t moduleIndex;
			json_t* moduleSlugJ;
			json_array_foreach(modulesJ, moduleIndex, moduleSlugJ) {
				std::string moduleSlug = json_string_value(moduleSlugJ);
				// Insert module in whitelist
				DEBUG("plugin %s module %s", pluginSlug, moduleSlug.c_str());
				moduleWhitelist[pluginSlug].insert(moduleSlug);
			}
		}

		settings::moduleWhitelist = moduleWhitelist;
	}

	updateStatus = "";
	refreshRequested = true;
}


bool hasUpdates() {
	for (auto& pair : updateInfos) {
		if (!pair.second.downloaded)
			return true;
	}
	return false;
}


void syncUpdate(std::string slug) {
	if (!updateMutex.try_lock())
		return;
	DEFER({updateMutex.unlock();});

	if (settings::token.empty())
		return;

	isSyncing = true;
	DEFER({isSyncing = false;});

	// Get the UpdateInfo object
	auto it = updateInfos.find(slug);
	if (it == updateInfos.end())
		return;
	UpdateInfo update = it->second;

	updateSlug = slug;
	DEFER({updateSlug = "";});

	// Set progress to 0%
	updateProgress = 0.f;
	DEFER({updateProgress = 0.f;});

	INFO("Downloading plugin %s v%s for %s", slug.c_str(), update.version.c_str(), APP_ARCH.c_str());

	// Get download URL
	std::string downloadUrl = API_URL + "/download";
	downloadUrl += "?slug=" + network::encodeUrl(slug);
	downloadUrl += "&version=" + network::encodeUrl(update.version);
	downloadUrl += "&arch=" + network::encodeUrl(APP_ARCH);

	// Get file path
	std::string packageFilename = slug + "-" + update.version + "-" + APP_ARCH + ".vcvplugin";
	std::string packagePath = system::join(plugin::pluginsPath, packageFilename);

	// Download plugin package
	if (!network::requestDownload(downloadUrl, packagePath, &updateProgress, getTokenCookies())) {
		WARN("Plugin %s download was unsuccessful", slug.c_str());
		return;
	}

	// updateInfos could possibly change in the checkUpdates() thread, so re-get the UpdateInfo to modify it.
	it = updateInfos.find(slug);
	if (it == updateInfos.end())
		return;
	it->second.downloaded = true;
}


void syncUpdates() {
	if (settings::token.empty())
		return;

	// updateInfos could possibly change in the checkUpdates() thread, but checkUpdates() will not execute if syncUpdate() is running, so the chance of the updateInfos map being modified while iterating is rare.
	auto updateInfosClone = updateInfos;
	for (auto& pair : updateInfosClone) {
		syncUpdate(pair.first);
	}
	restartRequested = true;
}


std::string appVersion;
std::string appDownloadUrl;
std::string appChangelogUrl;

std::string loginStatus;
std::map<std::string, UpdateInfo> updateInfos;
std::string updateStatus;
std::string updateSlug;
float updateProgress = 0.f;
bool isSyncing = false;
bool restartRequested = false;
bool refreshRequested = false;


} // namespace library
} // namespace rack
