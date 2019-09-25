#include <updater.hpp>
#include <settings.hpp>
#include <app/common.hpp>
#include <network.hpp>
#include <system.hpp>
#include <app.hpp>
#include <window.hpp>
#include <asset.hpp>
#include <thread>


namespace rack {
namespace updater {


std::string version;
std::string changelogUrl;
float progress = 0.f;
static std::string downloadUrl;


static void checkVersion() {
	std::string versionUrl = app::API_URL + "/version";
	json_t* resJ = network::requestJson(network::METHOD_GET, versionUrl, NULL);
	if (!resJ) {
		WARN("Request for version failed");
		return;
	}
	DEFER({
		json_decref(resJ);
	});

	json_t* versionJ = json_object_get(resJ, "version");
	if (versionJ)
		version = json_string_value(versionJ);

	json_t* changelogUrlJ = json_object_get(resJ, "changelogUrl");
	if (changelogUrlJ)
		changelogUrl = json_string_value(changelogUrlJ);

	json_t* downloadUrlsJ = json_object_get(resJ, "downloadUrls");
	if (downloadUrlsJ) {
		json_t* downloadUrlJ = json_object_get(downloadUrlsJ, app::APP_ARCH.c_str());
		if (downloadUrlJ)
			downloadUrl = json_string_value(downloadUrlJ);
	}
}


void init() {
	if (!settings::devMode) {
		std::thread t([] {
			checkVersion();
		});
		t.detach();
	}
}


void update() {
	if (downloadUrl == "")
		return;

	// Download update
	std::string filename = string::filename(network::urlPath(downloadUrl));
	std::string path = asset::user(filename);
	INFO("Downloading update %s to %s", downloadUrl.c_str(), path.c_str());
	network::requestDownload(downloadUrl, path, &progress);

#if defined ARCH_WIN
	// Launch the installer
	INFO("Launching update %s", path.c_str());
	system::runProcessDetached(path);
#elif defined ARCH_MAC
	std::string cmd;
	// std::string appPath = asset::userDir + "/Rack.app";
	// cmd = "rm -rf '" + appPath + "'";
	// std::system(cmd.c_str());
	// // Unzip app using Apple's unzipper, since Rack's unzipper doesn't handle the metadata stuff correctly.
	// cmd = "unzip -q '" + path + "' -d '" + asset::userDir + "'";
	// std::system(cmd.c_str());
	// // Open app in Finder
	// cmd = "open -R '" + appPath + "'";
	// std::system(cmd.c_str());

	// Open Archive Utility
	cmd = "open '" + path + "'";
	std::system(cmd.c_str());
#elif defined ARCH_LIN
	system::openFolder(asset::user(""));
#endif

	APP->window->close();
}


bool isUpdateAvailable() {
	return (version != "") && (version != app::APP_VERSION);
}


} // namespace updater
} // namespace rack
