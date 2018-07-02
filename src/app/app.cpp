#include "global_pre.hpp"
#include "app.hpp"
#include "util/request.hpp"
#include <thread>
#include "global_ui.hpp"


namespace rack {


#ifndef USE_VST2
static void checkVersion() {
	json_t *resJ = requestJson(METHOD_GET, global_ui->app.gApiHost + "/version", NULL);

	if (resJ) {
		json_t *versionJ = json_object_get(resJ, "version");
		if (versionJ) {
			const char *version = json_string_value(versionJ);
			if (version && version != global_ui->app.gApplicationVersion) {
				global_ui->app.gLatestVersion = version;
			}
		}
		json_decref(resJ);
	}
}
#endif // USE_VST2


void appInit(bool devMode) {
	global_ui->app.gRackScene = new RackScene();
	global_ui->ui.gScene = global_ui->app.gRackScene;

#ifndef USE_VST2
	// Request latest version from server
	if (!devMode && global_ui->app.gCheckVersion) {
		std::thread t(checkVersion);
		t.detach();
	}
#endif // USE_VST2
}

void appDestroy() {
	delete global_ui->app.gRackScene;
}


json_t *colorToJson(NVGcolor color) {
	json_t *colorJ = json_object();
	json_object_set_new(colorJ, "r", json_real(color.r));
	json_object_set_new(colorJ, "g", json_real(color.g));
	json_object_set_new(colorJ, "b", json_real(color.b));
	json_object_set_new(colorJ, "a", json_real(color.a));
	return colorJ;
}

NVGcolor jsonToColor(json_t *colorJ) {
	NVGcolor color;
	color.r = json_number_value(json_object_get(colorJ, "r"));
	color.g = json_number_value(json_object_get(colorJ, "g"));
	color.b = json_number_value(json_object_get(colorJ, "b"));
	color.a = json_number_value(json_object_get(colorJ, "a"));
	return color;
}


} // namespace rack
