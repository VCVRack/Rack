#include <app/common.hpp>
#include <settings.hpp>
#include <network.hpp>
#include <thread>


namespace rack {
namespace app {


std::string APP_NAME = "VCV Rack";
std::string APP_VERSION = TOSTRING(VERSION);
std::string APP_NEW_VERSION;
std::string API_URL = "https://api.vcvrack.com";

static void checkVersion() {
	std::string versionUrl = app::API_URL + "/version";
	json_t *versionResJ = network::requestJson(network::METHOD_GET, versionUrl, NULL);
	if (!versionResJ) {
		WARN("Request for version failed");
		return;
	}
	DEFER({
		json_decref(versionResJ);
	});

	json_t *versionJ = json_object_get(versionResJ, "version");
	if (versionJ)
		APP_NEW_VERSION = json_string_value(versionJ);
}

void init() {
	if (!settings::devMode) {
		std::thread t([] {
			checkVersion();
		});
		t.detach();
	}
}


} // namespace app
} // namespace rack
