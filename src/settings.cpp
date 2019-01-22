#include "settings.hpp"
#include "window.hpp"
#include "plugin.hpp"
#include "app/Scene.hpp"
#include "app/ModuleBrowser.hpp"
#include "engine/Engine.hpp"
#include "app.hpp"
#include <jansson.h>


namespace rack {
namespace settings {


static json_t *settingsToJson() {
	// root
	json_t *rootJ = json_object();

	// token
	json_t *tokenJ = json_string(plugin::token.c_str());
	json_object_set_new(rootJ, "token", tokenJ);

	if (!app()->window->isMaximized()) {
		// windowSize
		math::Vec windowSize = app()->window->getWindowSize();
		json_t *windowSizeJ = json_pack("[f, f]", windowSize.x, windowSize.y);
		json_object_set_new(rootJ, "windowSize", windowSizeJ);

		// windowPos
		math::Vec windowPos = app()->window->getWindowPos();
		json_t *windowPosJ = json_pack("[f, f]", windowPos.x, windowPos.y);
		json_object_set_new(rootJ, "windowPos", windowPosJ);
	}

	// cableOpacity
	json_t *cableOpacityJ = json_real(cableOpacity);
	json_object_set_new(rootJ, "cableOpacity", cableOpacityJ);

	// cableTension
	json_t *cableTensionJ = json_real(cableTension);
	json_object_set_new(rootJ, "cableTension", cableTensionJ);

	// zoom
	json_t *zoomJ = json_real(zoom);
	json_object_set_new(rootJ, "zoom", zoomJ);

	// allowCursorLock
	json_t *allowCursorLockJ = json_boolean(app()->window->allowCursorLock);
	json_object_set_new(rootJ, "allowCursorLock", allowCursorLockJ);

	// sampleRate
	json_t *sampleRateJ = json_real(app()->engine->getSampleRate());
	json_object_set_new(rootJ, "sampleRate", sampleRateJ);

	// patchPath
	json_t *patchPathJ = json_string(app()->scene->rackWidget->patchPath.c_str());
	json_object_set_new(rootJ, "patchPath", patchPathJ);

	// skipLoadOnLaunch
	if (skipLoadOnLaunch) {
		json_object_set_new(rootJ, "skipLoadOnLaunch", json_true());
	}

	// moduleBrowser
	json_object_set_new(rootJ, "moduleBrowser", moduleBrowserToJson());

	// powerMeter
	json_object_set_new(rootJ, "powerMeter", json_boolean(powerMeter));

	// checkVersion
	json_object_set_new(rootJ, "checkVersion", json_boolean(checkVersion));

	// paramTooltip
	json_object_set_new(rootJ, "paramTooltip", json_boolean(paramTooltip));

	return rootJ;
}

static void settingsFromJson(json_t *rootJ) {
	// token
	json_t *tokenJ = json_object_get(rootJ, "token");
	if (tokenJ)
		plugin::token = json_string_value(tokenJ);

	// windowSize
	json_t *windowSizeJ = json_object_get(rootJ, "windowSize");
	if (windowSizeJ) {
		double width, height;
		json_unpack(windowSizeJ, "[F, F]", &width, &height);
		app()->window->setWindowSize(math::Vec(width, height));
	}

	// windowPos
	json_t *windowPosJ = json_object_get(rootJ, "windowPos");
	if (windowPosJ) {
		double x, y;
		json_unpack(windowPosJ, "[F, F]", &x, &y);
		app()->window->setWindowPos(math::Vec(x, y));
	}

	// cableOpacity
	json_t *cableOpacityJ = json_object_get(rootJ, "cableOpacity");
	if (cableOpacityJ)
		cableOpacity = json_number_value(cableOpacityJ);

	// tension
	json_t *tensionJ = json_object_get(rootJ, "cableTension");
	if (tensionJ)
		cableTension = json_number_value(tensionJ);

	// zoom
	json_t *zoomJ = json_object_get(rootJ, "zoom");
	if (zoomJ)
		zoom = json_number_value(zoomJ);

	// allowCursorLock
	json_t *allowCursorLockJ = json_object_get(rootJ, "allowCursorLock");
	if (allowCursorLockJ)
		app()->window->allowCursorLock = json_is_true(allowCursorLockJ);

	// sampleRate
	json_t *sampleRateJ = json_object_get(rootJ, "sampleRate");
	if (sampleRateJ) {
		float sampleRate = json_number_value(sampleRateJ);
		app()->engine->setSampleRate(sampleRate);
	}

	// patchPath
	json_t *patchPathJ = json_object_get(rootJ, "patchPath");
	if (patchPathJ)
		app()->scene->rackWidget->patchPath = json_string_value(patchPathJ);

	// skipLoadOnLaunch
	json_t *skipLoadOnLaunchJ = json_object_get(rootJ, "skipLoadOnLaunch");
	if (skipLoadOnLaunchJ)
		skipLoadOnLaunch = json_boolean_value(skipLoadOnLaunchJ);

	// moduleBrowser
	json_t *moduleBrowserJ = json_object_get(rootJ, "moduleBrowser");
	if (moduleBrowserJ)
		moduleBrowserFromJson(moduleBrowserJ);

	// powerMeter
	json_t *powerMeterJ = json_object_get(rootJ, "powerMeter");
	if (powerMeterJ)
		powerMeter = json_boolean_value(powerMeterJ);

	// checkVersion
	json_t *checkVersionJ = json_object_get(rootJ, "checkVersion");
	if (checkVersionJ)
		checkVersion = json_boolean_value(checkVersionJ);

	// paramTooltip
	json_t *paramTooltipJ = json_object_get(rootJ, "paramTooltip");
	if (paramTooltipJ)
		paramTooltip = json_boolean_value(paramTooltipJ);
}


void save(std::string filename) {
	INFO("Saving settings %s", filename.c_str());
	json_t *rootJ = settingsToJson();
	if (rootJ) {
		FILE *file = fopen(filename.c_str(), "w");
		if (!file)
			return;

		json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		json_decref(rootJ);
		fclose(file);
	}
}

void load(std::string filename) {
	INFO("Loading settings %s", filename.c_str());
	FILE *file = fopen(filename.c_str(), "r");
	if (!file)
		return;

	json_error_t error;
	json_t *rootJ = json_loadf(file, 0, &error);
	if (rootJ) {
		settingsFromJson(rootJ);
		json_decref(rootJ);
	}
	else {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	}

	fclose(file);
}


float zoom = 1.0;
float cableOpacity = 0.5;
float cableTension = 0.5;
bool paramTooltip = false;
bool powerMeter = false;
bool lockModules = false;
bool checkVersion = true;
bool skipLoadOnLaunch = false;


} // namespace settings
} // namespace rack
