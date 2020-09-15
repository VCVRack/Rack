#include <thread>

#include <library.hpp>
#include <settings.hpp>
#include <app/common.hpp>
#include <network.hpp>
#include <system.hpp>
#include <context.hpp>
#include <window.hpp>
#include <asset.hpp>
#include <settings.hpp>
#include <plugin.hpp>


namespace rack {
namespace library {


static void queryAppUpdate();


void init() {
	if (settings::devMode)
		return;

	if (settings::checkAppUpdates) {
		std::thread t([&]() {
			queryAppUpdate();
		});
		t.detach();
	}

	std::thread t2([&] {
		queryUpdates();
	});
	t2.detach();
}


void destroy() {
}


static void queryAppUpdate() {
	std::string versionUrl = API_URL + "/version";
	json_t* resJ = network::requestJson(network::METHOD_GET, versionUrl, NULL);
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


void logIn(const std::string& email, const std::string& password) {
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
	DEFER({
		json_decref(resJ);
	});

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
	queryUpdates();
}


void logOut() {
	settings::token = "";
	updates.clear();
}


void queryUpdates() {
	if (settings::token.empty())
		return;

	updates.clear();
	updateStatus = "Querying for updates...";

	// Get user's plugins list
	std::string pluginsUrl = API_URL + "/plugins";
	network::CookieMap cookies;
	cookies["token"] = settings::token;
	json_t* pluginsResJ = network::requestJson(network::METHOD_GET, pluginsUrl, NULL, cookies);
	if (!pluginsResJ) {
		WARN("Request for user's plugins failed");
		updateStatus = "Could not query updates";
		return;
	}
	DEFER({
		json_decref(pluginsResJ);
	});

	json_t* errorJ = json_object_get(pluginsResJ, "error");
	if (errorJ) {
		WARN("Request for user's plugins returned an error: %s", json_string_value(errorJ));
		updateStatus = "Could not query updates";
		return;
	}

	// Get library manifests
	std::string manifestsUrl = API_URL + "/library/manifests";
	json_t* manifestsReq = json_object();
	json_object_set(manifestsReq, "version", json_string(API_VERSION.c_str()));
	json_t* manifestsResJ = network::requestJson(network::METHOD_GET, manifestsUrl, manifestsReq);
	json_decref(manifestsReq);
	if (!manifestsResJ) {
		WARN("Request for library manifests failed");
		updateStatus = "Could not query updates";
		return;
	}
	DEFER({
		json_decref(manifestsResJ);
	});

	json_t* manifestsJ = json_object_get(manifestsResJ, "manifests");
	json_t* pluginsJ = json_object_get(pluginsResJ, "plugins");

	size_t pluginIndex;
	json_t* pluginJ;
	json_array_foreach(pluginsJ, pluginIndex, pluginJ) {
		Update update;
		// Get plugin manifest
		update.pluginSlug = json_string_value(pluginJ);
		json_t* manifestJ = json_object_get(manifestsJ, update.pluginSlug.c_str());
		if (!manifestJ) {
			WARN("VCV account has plugin %s but no manifest was found", update.pluginSlug.c_str());
			continue;
		}

		// Get plugin name
		json_t* nameJ = json_object_get(manifestJ, "name");
		if (nameJ)
			update.pluginName = json_string_value(nameJ);

		// Get version
		json_t* versionJ = json_object_get(manifestJ, "version");
		if (!versionJ) {
			WARN("Plugin %s has no version in manifest", update.pluginSlug.c_str());
			continue;
		}
		update.version = json_string_value(versionJ);

		// Check if update is needed
		plugin::Plugin* p = plugin::getPlugin(update.pluginSlug);
		if (p && p->version == update.version)
			continue;

		// Check status
		json_t* statusJ = json_object_get(manifestJ, "status");
		if (!statusJ)
			continue;
		std::string status = json_string_value(statusJ);
		if (status != "available")
			continue;

		// Get changelog URL
		json_t* changelogUrlJ = json_object_get(manifestJ, "changelogUrl");
		if (changelogUrlJ) {
			update.changelogUrl = json_string_value(changelogUrlJ);
		}

		updates.push_back(update);
	}


	// Get module whitelist
	{
		std::string whitelistUrl = API_URL + "/moduleWhitelist";
		json_t* whitelistResJ = network::requestJson(network::METHOD_GET, whitelistUrl, NULL, cookies);
		if (!whitelistResJ) {
			WARN("Request for module whitelist failed");
			updateStatus = "Could not query updates";
			return;
		}
		DEFER({
			json_decref(whitelistResJ);
		});

		std::map<std::string, std::set<std::string>> moduleWhitelist;
		json_t* pluginsJ = json_object_get(whitelistResJ, "plugins");

		// Iterate plugins
		const char* pluginSlug;
		json_t* modulesJ;
		json_object_foreach(pluginsJ, pluginSlug, modulesJ) {
			// Iterate modules in plugin
			size_t moduleIndex;
			json_t* moduleSlugJ;
			json_array_foreach(modulesJ, moduleIndex, moduleSlugJ) {
				std::string moduleSlug = json_string_value(moduleSlugJ);
				// Insert module in whitelist
				moduleWhitelist[pluginSlug].insert(moduleSlug);
			}
		}

		settings::moduleWhitelist = moduleWhitelist;
	}

	updateStatus = "";
}


bool hasUpdates() {
	for (Update& update : updates) {
		if (update.progress < 1.f)
			return true;
	}
	return false;
}


static bool isSyncingUpdate = false;
static bool isSyncingUpdates = false;


void syncUpdate(Update* update) {
	isSyncingUpdate = true;
	DEFER({
		isSyncingUpdate = false;
	});

	std::string downloadUrl = API_URL + "/download";
	downloadUrl += "?slug=" + network::encodeUrl(update->pluginSlug);
	downloadUrl += "&version=" + network::encodeUrl(update->version);
	downloadUrl += "&arch=" + network::encodeUrl(APP_ARCH);

	network::CookieMap cookies;
	cookies["token"] = settings::token;

	INFO("Downloading plugin %s %s %s", update->pluginSlug.c_str(), update->version.c_str(), APP_ARCH.c_str());

	// Download zip
	std::string pluginDest = system::join(asset::pluginsPath, update->pluginSlug + ".zip");
	if (!network::requestDownload(downloadUrl, pluginDest, &update->progress, cookies)) {
		WARN("Plugin %s download was unsuccessful", update->pluginSlug.c_str());
		return;
	}
}


void syncUpdates() {
	isSyncingUpdates = true;
	DEFER({
		isSyncingUpdates = false;
	});

	if (settings::token.empty())
		return;

	for (Update& update : updates) {
		if (update.progress < 1.f)
			syncUpdate(&update);
	}
	restartRequested = true;
}


bool isSyncing() {
	return isSyncingUpdate || isSyncingUpdates;
}


std::string loginStatus;
std::vector<Update> updates;
std::string updateStatus;
bool restartRequested = false;

std::string appVersion;
std::string appDownloadUrl;
std::string appChangelogUrl;


} // namespace library
} // namespace rack
