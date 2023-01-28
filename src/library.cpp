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
static std::mutex timeoutMutex;
static std::condition_variable updateCv;


void init() {
	if (!settings::autoCheckUpdates)
		return;
	// Dev mode is typically used when Rack or plugins are compiled from source, so updating might overwrite assets.
	if (settings::devMode)
		return;
	// Safe mode disables plugin loading, so Rack will unnecessarily try to sync all plugins.
	if (settings::safeMode)
		return;

	std::thread t([&]() {
		system::setThreadName("Library");
		// Wait a few seconds before updating in case library is destroyed immediately afterwards
		{
			std::unique_lock<std::mutex> lock(timeoutMutex);
			if (updateCv.wait_for(lock, std::chrono::duration<double>(4.0)) != std::cv_status::timeout)
				return;
		}

		checkAppUpdate();
		checkUpdates();
	});
	t.detach();
}


void destroy() {
	// Wait until all library threads are finished
	updateCv.notify_all();
	std::lock_guard<std::mutex> timeoutLock(timeoutMutex);
	std::lock_guard<std::mutex> appUpdateLock(appUpdateMutex);
	std::lock_guard<std::mutex> updateLock(updateMutex);
}


void checkAppUpdate() {
	if (!appUpdateMutex.try_lock())
		return;
	DEFER({appUpdateMutex.unlock();});

	std::string versionUrl = API_URL + "/version";
	json_t* reqJ = json_object();
	json_object_set_new(reqJ, "edition", json_string(APP_EDITION.c_str()));
	DEFER({json_decref(reqJ);});

	json_t* resJ = network::requestJson(network::METHOD_GET, versionUrl, reqJ);
	if (!resJ) {
		WARN("Request for version failed");
		return;
	}
	DEFER({json_decref(resJ);});

	json_t* versionJ = json_object_get(resJ, "version");
	if (versionJ) {
		std::string appVersion = json_string_value(versionJ);
		// Check if app version is more recent than current version
		if (string::Version(APP_VERSION) < string::Version(appVersion))
			library::appVersion = appVersion;
	}

	json_t* changelogUrlJ = json_object_get(resJ, "changelogUrl");
	if (changelogUrlJ)
		appChangelogUrl = json_string_value(changelogUrlJ);

	json_t* downloadUrlsJ = json_object_get(resJ, "downloadUrls");
	if (downloadUrlsJ) {
		std::string arch = APP_OS + "-" + APP_CPU;
		json_t* downloadUrlJ = json_object_get(downloadUrlsJ, arch.c_str());
		if (downloadUrlJ)
			appDownloadUrl = json_string_value(downloadUrlJ);
	}
}


bool isAppUpdateAvailable() {
	return (appVersion != "");
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
	json_object_set_new(reqJ, "email", json_string(email.c_str()));
	json_object_set_new(reqJ, "password", json_string(password.c_str()));
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
		WARN("Request for user account failed");
		updateStatus = "Could not query user account";
		return;
	}
	DEFER({json_decref(userResJ);});

	json_t* userErrorJ = json_object_get(userResJ, "error");
	if (userErrorJ) {
		std::string userError = json_string_value(userErrorJ);
		WARN("Request for user account error: %s", userError.c_str());
		// Unset token
		settings::token = "";
		refreshRequested = true;
		return;
	}

	// Get library manifests
	std::string manifestsUrl = API_URL + "/library/manifests";
	json_t* manifestsReq = json_object();
	json_object_set_new(manifestsReq, "version", json_string(APP_VERSION_MAJOR.c_str()));
	json_t* manifestsResJ = network::requestJson(network::METHOD_GET, manifestsUrl, manifestsReq);
	json_decref(manifestsReq);
	if (!manifestsResJ) {
		WARN("Request for library manifests failed");
		updateStatus = "Could not query plugin manifests";
		return;
	}
	DEFER({json_decref(manifestsResJ);});

	// Get user's modules
	std::string modulesUrl = API_URL + "/modules";
	json_t* modulesResJ = network::requestJson(network::METHOD_GET, modulesUrl, NULL, getTokenCookies());
	if (!modulesResJ) {
		WARN("Request for user's modules failed");
		updateStatus = "Could not query user's modules";
		return;
	}
	DEFER({json_decref(modulesResJ);});

	json_t* manifestsJ = json_object_get(manifestsResJ, "manifests");
	json_t* pluginsJ = json_object_get(modulesResJ, "modules");

	const char* modulesKey;
	json_t* modulesJ;
	json_object_foreach(pluginsJ, modulesKey, modulesJ) {
		std::string pluginSlug = modulesKey;
		// Get plugin manifest
		json_t* manifestJ = json_object_get(manifestsJ, pluginSlug.c_str());
		if (!manifestJ) {
			// Skip plugin silently
			continue;
		}

		// Don't replace existing UpdateInfo, even if version is newer.
		// This keeps things sane and ensures that only one version of each plugin is downloaded to `plugins/` at a time.
		auto it = updateInfos.find(pluginSlug);
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
			// WARN("Plugin %s has no version in manifest", pluginSlug.c_str());
			continue;
		}
		update.version = json_string_value(versionJ);
		// Reject plugins with ABI mismatch
		if (!string::startsWith(update.version, APP_VERSION_MAJOR + ".")) {
			continue;
		}

		// Check that update is needed
		plugin::Plugin* p = plugin::getPlugin(pluginSlug);
		if (p) {
			if (update.version == p->version)
				continue;
			if (string::Version(update.version) < string::Version(p->version))
				continue;
		}

		// Check that plugin is available for this arch
		json_t* archesJ = json_object_get(manifestJ, "arches");
		if (!archesJ)
			continue;
		std::string arch = APP_OS + "-" + APP_CPU;
		json_t* archJ = json_object_get(archesJ, arch.c_str());
		if (!json_boolean_value(archJ))
			continue;

		// Get changelog URL
		json_t* changelogUrlJ = json_object_get(manifestJ, "changelogUrl");
		if (changelogUrlJ)
			update.changelogUrl = json_string_value(changelogUrlJ);

		// Add update to updates map
		updateInfos[pluginSlug] = update;
	}

	// Merge module whitelist
	{
		// Clone plugin slugs from settings to temporary whitelist.
		// This makes existing plugins entirely hidden if removed from user's VCV account.
		std::map<std::string, settings::PluginWhitelist> moduleWhitelist;
		for (const auto& pluginPair : settings::moduleWhitelist) {
			std::string pluginSlug = pluginPair.first;
			moduleWhitelist[pluginSlug] = settings::PluginWhitelist();
		}

		// Iterate plugins
		const char* modulesKey;
		json_t* modulesJ;
		json_object_foreach(pluginsJ, modulesKey, modulesJ) {
			std::string pluginSlug = modulesKey;
			settings::PluginWhitelist& pw = moduleWhitelist[pluginSlug];

			// If value is "true", plugin is subscribed
			if (json_is_true(modulesJ)) {
				pw.subscribed = true;
				continue;
			}

			// Iterate modules in plugin
			size_t moduleIndex;
			json_t* moduleSlugJ;
			json_array_foreach(modulesJ, moduleIndex, moduleSlugJ) {
				std::string moduleSlug = json_string_value(moduleSlugJ);
				// Insert module in whitelist
				pw.moduleSlugs.insert(moduleSlug);
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

	INFO("Downloading plugin %s v%s for %s-%s", slug.c_str(), update.version.c_str(), APP_OS.c_str(), APP_CPU.c_str());

	// Get download URL
	std::string downloadUrl = API_URL + "/download";
	downloadUrl += "?slug=" + network::encodeUrl(slug);
	downloadUrl += "&version=" + network::encodeUrl(update.version);
	downloadUrl += "&arch=" + network::encodeUrl(APP_OS + "-" + APP_CPU);

	// Get file path
	std::string packageFilename = slug + "-" + update.version + "-" + APP_OS + "-" + APP_CPU + ".vcvplugin";
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
