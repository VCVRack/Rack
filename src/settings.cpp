#include "settings.hpp"
#include "app.hpp"
#include "window.hpp"
#include "engine.hpp"
#include "plugin.hpp"
#include <jansson.h>


namespace rack {


bool gSkipAutosaveOnLaunch = false;


static json_t *settingsToJson() {
	// root
	json_t *rootJ = json_object();

	// token
	json_t *tokenJ = json_string(gToken.c_str());
	json_object_set_new(rootJ, "token", tokenJ);

	if (!windowIsMaximized()) {
		// windowSize
		Vec windowSize = windowGetWindowSize();
		json_t *windowSizeJ = json_pack("[f, f]", windowSize.x, windowSize.y);
		json_object_set_new(rootJ, "windowSize", windowSizeJ);

		// windowPos
		Vec windowPos = windowGetWindowPos();
		json_t *windowPosJ = json_pack("[f, f]", windowPos.x, windowPos.y);
		json_object_set_new(rootJ, "windowPos", windowPosJ);
	}

	// opacity
	float opacity = gToolbar->wireOpacitySlider->value;
	json_t *opacityJ = json_real(opacity);
	json_object_set_new(rootJ, "wireOpacity", opacityJ);

	// tension
	float tension = gToolbar->wireTensionSlider->value;
	json_t *tensionJ = json_real(tension);
	json_object_set_new(rootJ, "wireTension", tensionJ);

	// zoom
	float zoom = gRackScene->zoomWidget->zoom;
	json_t *zoomJ = json_real(zoom);
	json_object_set_new(rootJ, "zoom", zoomJ);

	// allowCursorLock
	json_t *allowCursorLockJ = json_boolean(gAllowCursorLock);
	json_object_set_new(rootJ, "allowCursorLock", allowCursorLockJ);

	// sampleRate
	json_t *sampleRateJ = json_real(engineGetSampleRate());
	json_object_set_new(rootJ, "sampleRate", sampleRateJ);

	// lastPath
	json_t *lastPathJ = json_string(gRackWidget->lastPath.c_str());
	json_object_set_new(rootJ, "lastPath", lastPathJ);

	// skipAutosaveOnLaunch
	if (gSkipAutosaveOnLaunch) {
		json_object_set_new(rootJ, "skipAutosaveOnLaunch", json_true());
	}

	// moduleBrowser
	json_object_set_new(rootJ, "moduleBrowser", appModuleBrowserToJson());

	// powerMeter
	json_object_set_new(rootJ, "powerMeter", json_boolean(gPowerMeter));

	// checkVersion
	json_object_set_new(rootJ, "checkVersion", json_boolean(gCheckVersion));

	return rootJ;
}

static void settingsFromJson(json_t *rootJ) {
	// token
	json_t *tokenJ = json_object_get(rootJ, "token");
	if (tokenJ)
		gToken = json_string_value(tokenJ);

	// windowSize
	json_t *windowSizeJ = json_object_get(rootJ, "windowSize");
	if (windowSizeJ) {
		double width, height;
		json_unpack(windowSizeJ, "[F, F]", &width, &height);
		windowSetWindowSize(Vec(width, height));
	}

	// windowPos
	json_t *windowPosJ = json_object_get(rootJ, "windowPos");
	if (windowPosJ) {
		double x, y;
		json_unpack(windowPosJ, "[F, F]", &x, &y);
		windowSetWindowPos(Vec(x, y));
	}

	// opacity
	json_t *opacityJ = json_object_get(rootJ, "wireOpacity");
	if (opacityJ)
		gToolbar->wireOpacitySlider->value = json_number_value(opacityJ);

	// tension
	json_t *tensionJ = json_object_get(rootJ, "wireTension");
	if (tensionJ)
		gToolbar->wireTensionSlider->value = json_number_value(tensionJ);

	// zoom
	json_t *zoomJ = json_object_get(rootJ, "zoom");
	if (zoomJ) {
		gRackScene->zoomWidget->setZoom(clamp((float) json_number_value(zoomJ), 0.25f, 4.0f));
		gToolbar->zoomSlider->setValue(json_number_value(zoomJ) * 100.0);
	}

	// allowCursorLock
	json_t *allowCursorLockJ = json_object_get(rootJ, "allowCursorLock");
	if (allowCursorLockJ)
		gAllowCursorLock = json_is_true(allowCursorLockJ);

	// sampleRate
	json_t *sampleRateJ = json_object_get(rootJ, "sampleRate");
	if (sampleRateJ) {
		float sampleRate = json_number_value(sampleRateJ);
		engineSetSampleRate(sampleRate);
	}

	// lastPath
	json_t *lastPathJ = json_object_get(rootJ, "lastPath");
	if (lastPathJ)
		gRackWidget->lastPath = json_string_value(lastPathJ);

	// skipAutosaveOnLaunch
	json_t *skipAutosaveOnLaunchJ = json_object_get(rootJ, "skipAutosaveOnLaunch");
	if (skipAutosaveOnLaunchJ)
		gSkipAutosaveOnLaunch = json_boolean_value(skipAutosaveOnLaunchJ);

	// moduleBrowser
	json_t *moduleBrowserJ = json_object_get(rootJ, "moduleBrowser");
	if (moduleBrowserJ)
		appModuleBrowserFromJson(moduleBrowserJ);

	// powerMeter
	json_t *powerMeterJ = json_object_get(rootJ, "powerMeter");
	if (powerMeterJ)
		gPowerMeter = json_boolean_value(powerMeterJ);

	// checkVersion
	json_t *checkVersionJ = json_object_get(rootJ, "checkVersion");
	if (checkVersionJ)
		gCheckVersion = json_boolean_value(checkVersionJ);
}


void settingsSave(std::string filename) {
	info("Saving settings %s", filename.c_str());
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

void settingsLoad(std::string filename) {
	info("Loading settings %s", filename.c_str());
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
		warn("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	}

	fclose(file);
}


} // namespace rack
