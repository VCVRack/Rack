#include "settings.hpp"
#include "window.hpp"
#include "plugin.hpp"
#include "app/Scene.hpp"
#include "app/ModuleBrowser.hpp"
#include "engine/Engine.hpp"
#include "app.hpp"
#include "patch.hpp"
#include <jansson.h>


namespace rack {


json_t *Settings::toJson() {
	json_t *rootJ = json_object();

	json_object_set_new(rootJ, "token", json_string(token.c_str()));

	json_t *windowSizeJ = json_pack("[f, f]", windowSize.x, windowSize.y);
	json_object_set_new(rootJ, "windowSize", windowSizeJ);

	json_t *windowPosJ = json_pack("[f, f]", windowPos.x, windowPos.y);
	json_object_set_new(rootJ, "windowPos", windowPosJ);

	json_object_set_new(rootJ, "zoom", json_real(zoom));

	json_object_set_new(rootJ, "cableOpacity", json_real(cableOpacity));

	json_object_set_new(rootJ, "cableTension", json_real(cableTension));

	json_object_set_new(rootJ, "allowCursorLock", json_boolean(allowCursorLock));

	json_object_set_new(rootJ, "sampleRate", json_real(sampleRate));

	json_object_set_new(rootJ, "threadCount", json_integer(threadCount));

	json_object_set_new(rootJ, "paramTooltip", json_boolean(paramTooltip));

	json_object_set_new(rootJ, "cpuMeter", json_boolean(cpuMeter));

	json_object_set_new(rootJ, "lockModules", json_boolean(lockModules));

	json_object_set_new(rootJ, "checkVersion", json_boolean(checkVersion));

	json_object_set_new(rootJ, "frameRateLimit", json_real(frameRateLimit));

	json_object_set_new(rootJ, "frameRateSync", json_boolean(frameRateSync));

	if (skipLoadOnLaunch) {
		json_object_set_new(rootJ, "skipLoadOnLaunch", json_true());
	}

	json_object_set_new(rootJ, "patchPath", json_string(patchPath.c_str()));

	json_object_set_new(rootJ, "moduleBrowser", app::moduleBrowserToJson());

	return rootJ;
}

void Settings::fromJson(json_t *rootJ) {
	json_t *tokenJ = json_object_get(rootJ, "token");
	if (tokenJ)
		token = json_string_value(tokenJ);

	json_t *windowSizeJ = json_object_get(rootJ, "windowSize");
	if (windowSizeJ) {
		double x, y;
		json_unpack(windowSizeJ, "[F, F]", &x, &y);
		windowSize = math::Vec(x, y);
	}

	json_t *windowPosJ = json_object_get(rootJ, "windowPos");
	if (windowPosJ) {
		double x, y;
		json_unpack(windowPosJ, "[F, F]", &x, &y);
		windowPos = math::Vec(x, y);
	}

	json_t *zoomJ = json_object_get(rootJ, "zoom");
	if (zoomJ)
		zoom = json_number_value(zoomJ);

	json_t *cableOpacityJ = json_object_get(rootJ, "cableOpacity");
	if (cableOpacityJ)
		cableOpacity = json_number_value(cableOpacityJ);

	json_t *tensionJ = json_object_get(rootJ, "cableTension");
	if (tensionJ)
		cableTension = json_number_value(tensionJ);

	json_t *allowCursorLockJ = json_object_get(rootJ, "allowCursorLock");
	if (allowCursorLockJ)
		allowCursorLock = json_is_true(allowCursorLockJ);

	json_t *sampleRateJ = json_object_get(rootJ, "sampleRate");
	if (sampleRateJ)
		sampleRate = json_number_value(sampleRateJ);

	json_t *threadCountJ = json_object_get(rootJ, "threadCount");
	if (threadCountJ)
		threadCount = json_integer_value(threadCountJ);

	json_t *paramTooltipJ = json_object_get(rootJ, "paramTooltip");
	if (paramTooltipJ)
		paramTooltip = json_boolean_value(paramTooltipJ);

	json_t *cpuMeterJ = json_object_get(rootJ, "cpuMeter");
	if (cpuMeterJ)
		cpuMeter = json_boolean_value(cpuMeterJ);

	json_t *lockModulesJ = json_object_get(rootJ, "lockModules");
	if (lockModulesJ)
		lockModules = json_boolean_value(lockModulesJ);

	json_t *checkVersionJ = json_object_get(rootJ, "checkVersion");
	if (checkVersionJ)
		checkVersion = json_boolean_value(checkVersionJ);

	json_t *frameRateLimitJ = json_object_get(rootJ, "frameRateLimit");
	if (frameRateLimitJ)
		frameRateLimit = json_number_value(frameRateLimitJ);

	json_t *frameRateSyncJ = json_object_get(rootJ, "frameRateSync");
	if (frameRateSyncJ)
		frameRateSync = json_boolean_value(frameRateSyncJ);

	json_t *skipLoadOnLaunchJ = json_object_get(rootJ, "skipLoadOnLaunch");
	if (skipLoadOnLaunchJ)
		skipLoadOnLaunch = json_boolean_value(skipLoadOnLaunchJ);

	json_t *patchPathJ = json_object_get(rootJ, "patchPath");
	if (patchPathJ)
		patchPath = json_string_value(patchPathJ);

	json_t *moduleBrowserJ = json_object_get(rootJ, "moduleBrowser");
	if (moduleBrowserJ)
		app::moduleBrowserFromJson(moduleBrowserJ);
}

void Settings::save(std::string filename) {
	INFO("Saving settings %s", filename.c_str());
	json_t *rootJ = toJson();
	if (rootJ) {
		FILE *file = fopen(filename.c_str(), "w");
		if (!file)
			return;

		json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		json_decref(rootJ);
		fclose(file);
	}
}

void Settings::load(std::string filename) {
	INFO("Loading settings %s", filename.c_str());
	FILE *file = fopen(filename.c_str(), "r");
	if (!file)
		return;

	json_error_t error;
	json_t *rootJ = json_loadf(file, 0, &error);
	if (rootJ) {
		fromJson(rootJ);
		json_decref(rootJ);
	}
	else {
		WARN("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	}

	fclose(file);
}


Settings settings;


} // namespace rack
