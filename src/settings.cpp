#include "settings.hpp"
#include "app.hpp"
#include "gui.hpp"
#include "engine.hpp"
#include "plugin.hpp"
#include <jansson.h>


namespace rack {


static json_t *settingsToJson() {
	// root
	json_t *rootJ = json_object();

	// token
	json_t *tokenJ = json_string(gToken.c_str());
	json_object_set_new(rootJ, "token", tokenJ);

	if (!guiIsMaximized()) {
		// windowSize
		Vec windowSize = guiGetWindowSize();
		json_t *windowSizeJ = json_pack("[f, f]", windowSize.x, windowSize.y);
		json_object_set_new(rootJ, "windowSize", windowSizeJ);

		// windowPos
		Vec windowPos = guiGetWindowPos();
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

	// plugLight
	json_t *plugLightJ = json_boolean(gToolbar->plugLightButton->value > 0.0);
	json_object_set_new(rootJ, "plugLight", plugLightJ);

	// lastPath
	json_t *lastPathJ = json_string(gRackWidget->lastPath.c_str());
	json_object_set_new(rootJ, "lastPath", lastPathJ);

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
		guiSetWindowSize(Vec(width, height));
	}

	// windowPos
	json_t *windowPosJ = json_object_get(rootJ, "windowPos");
	if (windowPosJ) {
		double x, y;
		json_unpack(windowPosJ, "[F, F]", &x, &y);
		guiSetWindowPos(Vec(x, y));
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
		gRackScene->zoomWidget->setZoom(clampf(json_number_value(zoomJ), 0.25, 4.0));
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

	// plugLight
	json_t *plugLightJ = json_object_get(rootJ, "plugLight");
	if (plugLightJ)
		gToolbar->plugLightButton->setValue(json_is_true(plugLightJ) ? 1.0 : 0.0);

	// lastPath
	json_t *lastPathJ = json_object_get(rootJ, "lastPath");
	if (lastPathJ)
		gRackWidget->lastPath = json_string_value(lastPathJ);
}


void settingsSave(std::string filename) {
	log(INFO, "Saving settings %s", filename.c_str());
	FILE *file = fopen(filename.c_str(), "w");
	if (!file)
		return;

	json_t *rootJ = settingsToJson();
	if (rootJ) {
		json_dumpf(rootJ, file, JSON_INDENT(2));
		json_decref(rootJ);
	}

	fclose(file);
}

void settingsLoad(std::string filename) {
	log(INFO, "Loading settings %s", filename.c_str());
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
		log(WARN, "JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
	}

	fclose(file);
}


} // namespace rack
