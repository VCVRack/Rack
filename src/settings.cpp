#include <settings.hpp>
#include <window.hpp>
#include <plugin.hpp>
#include <app/Scene.hpp>
#include <app/ModuleBrowser.hpp>
#include <engine/Engine.hpp>
#include <app.hpp>
#include <patch.hpp>
#include <jansson.h>


namespace rack {
namespace settings {


bool devMode = false;
bool headless = false;
std::string token;
math::Vec windowSize;
math::Vec windowPos;
float zoom = 0.0;
bool invertZoom = false;
float cableOpacity = 0.5;
float cableTension = 0.5;
KnobMode knobMode = KNOB_MODE_LINEAR_LOCKED;
float knobLinearSensitivity = 0.001f;
float sampleRate = 44100.0;
int threadCount = 1;
bool paramTooltip = true;
bool cpuMeter = false;
bool lockModules = false;
#if defined ARCH_MAC
	// Most Mac GPUs can't handle rendering the screen every frame, so use ~30 Hz by default.
	int frameSwapInterval = 2;
#else
	int frameSwapInterval = 1;
#endif
float autosavePeriod = 15.0;
bool skipLoadOnLaunch = false;
std::string patchPath;
std::vector<NVGcolor> cableColors = {
	nvgRGB(0xc9, 0xb7, 0x0e), // yellow
	nvgRGB(0x0c, 0x8e, 0x15), // green
	nvgRGB(0xc9, 0x18, 0x47), // red
	nvgRGB(0x09, 0x86, 0xad), // blue
};
std::map<std::string, std::vector<std::string>> moduleWhitelist = {};


json_t* toJson() {
	json_t* rootJ = json_object();

	json_object_set_new(rootJ, "token", json_string(token.c_str()));

	json_t* windowSizeJ = json_pack("[f, f]", windowSize.x, windowSize.y);
	json_object_set_new(rootJ, "windowSize", windowSizeJ);

	json_t* windowPosJ = json_pack("[f, f]", windowPos.x, windowPos.y);
	json_object_set_new(rootJ, "windowPos", windowPosJ);

	json_object_set_new(rootJ, "zoom", json_real(zoom));

	json_object_set_new(rootJ, "invertZoom", json_boolean(invertZoom));

	json_object_set_new(rootJ, "cableOpacity", json_real(cableOpacity));

	json_object_set_new(rootJ, "cableTension", json_real(cableTension));

	json_object_set_new(rootJ, "knobMode", json_integer((int) knobMode));

	json_object_set_new(rootJ, "knobLinearSensitivity", json_real(knobLinearSensitivity));

	json_object_set_new(rootJ, "sampleRate", json_real(sampleRate));

	json_object_set_new(rootJ, "threadCount", json_integer(threadCount));

	json_object_set_new(rootJ, "paramTooltip", json_boolean(paramTooltip));

	json_object_set_new(rootJ, "cpuMeter", json_boolean(cpuMeter));

	json_object_set_new(rootJ, "lockModules", json_boolean(lockModules));

	json_object_set_new(rootJ, "frameSwapInterval", json_integer(frameSwapInterval));

	json_object_set_new(rootJ, "autosavePeriod", json_real(autosavePeriod));

	if (skipLoadOnLaunch)
		json_object_set_new(rootJ, "skipLoadOnLaunch", json_boolean(true));

	json_object_set_new(rootJ, "patchPath", json_string(patchPath.c_str()));

	json_t* cableColorsJ = json_array();
	for (NVGcolor cableColor : cableColors) {
		std::string colorStr = color::toHexString(cableColor);
		json_array_append_new(cableColorsJ, json_string(colorStr.c_str()));
	}
	json_object_set_new(rootJ, "cableColors", cableColorsJ);

	json_t* moduleWhitelistJ = json_object();
	for (const auto& pair : moduleWhitelist) {
		json_t* moduleSlugsJ = json_array();
		for (const std::string& moduleSlug : pair.second) {
			json_array_append_new(moduleSlugsJ, json_string(moduleSlug.c_str()));
		}
		json_object_set_new(moduleWhitelistJ, pair.first.c_str(), moduleSlugsJ);
	}
	json_object_set_new(rootJ, "moduleWhitelist", moduleWhitelistJ);

	return rootJ;
}

void fromJson(json_t* rootJ) {
	json_t* tokenJ = json_object_get(rootJ, "token");
	if (tokenJ)
		token = json_string_value(tokenJ);

	json_t* windowSizeJ = json_object_get(rootJ, "windowSize");
	if (windowSizeJ) {
		double x, y;
		json_unpack(windowSizeJ, "[F, F]", &x, &y);
		windowSize = math::Vec(x, y);
	}

	json_t* windowPosJ = json_object_get(rootJ, "windowPos");
	if (windowPosJ) {
		double x, y;
		json_unpack(windowPosJ, "[F, F]", &x, &y);
		windowPos = math::Vec(x, y);
	}

	json_t* zoomJ = json_object_get(rootJ, "zoom");
	if (zoomJ)
		zoom = json_number_value(zoomJ);

	json_t* invertZoomJ = json_object_get(rootJ, "invertZoom");
	if (invertZoomJ)
		invertZoom = json_boolean_value(invertZoomJ);

	json_t* cableOpacityJ = json_object_get(rootJ, "cableOpacity");
	if (cableOpacityJ)
		cableOpacity = json_number_value(cableOpacityJ);

	json_t* cableTensionJ = json_object_get(rootJ, "cableTension");
	if (cableTensionJ)
		cableTension = json_number_value(cableTensionJ);

	// legacy v1
	json_t* allowCursorLockJ = json_object_get(rootJ, "allowCursorLock");
	if (allowCursorLockJ) {
		if (json_is_false(allowCursorLockJ))
			knobMode = KNOB_MODE_LINEAR;
	}

	json_t* knobModeJ = json_object_get(rootJ, "knobMode");
	if (knobModeJ)
		knobMode = (KnobMode) json_integer_value(knobModeJ);

	json_t* knobLinearSensitivityJ = json_object_get(rootJ, "knobLinearSensitivity");
	if (knobLinearSensitivityJ)
		knobLinearSensitivity = json_number_value(knobLinearSensitivityJ);

	json_t* sampleRateJ = json_object_get(rootJ, "sampleRate");
	if (sampleRateJ)
		sampleRate = json_number_value(sampleRateJ);

	json_t* threadCountJ = json_object_get(rootJ, "threadCount");
	if (threadCountJ)
		threadCount = json_integer_value(threadCountJ);

	json_t* paramTooltipJ = json_object_get(rootJ, "paramTooltip");
	if (paramTooltipJ)
		paramTooltip = json_boolean_value(paramTooltipJ);

	json_t* cpuMeterJ = json_object_get(rootJ, "cpuMeter");
	if (cpuMeterJ)
		cpuMeter = json_boolean_value(cpuMeterJ);

	json_t* lockModulesJ = json_object_get(rootJ, "lockModules");
	if (lockModulesJ)
		lockModules = json_boolean_value(lockModulesJ);

	json_t* frameSwapIntervalJ = json_object_get(rootJ, "frameSwapInterval");
	if (frameSwapIntervalJ)
		frameSwapInterval = json_integer_value(frameSwapIntervalJ);

	json_t* autosavePeriodJ = json_object_get(rootJ, "autosavePeriod");
	if (autosavePeriodJ)
		autosavePeriod = json_number_value(autosavePeriodJ);

	json_t* skipLoadOnLaunchJ = json_object_get(rootJ, "skipLoadOnLaunch");
	if (skipLoadOnLaunchJ)
		skipLoadOnLaunch = json_boolean_value(skipLoadOnLaunchJ);

	json_t* patchPathJ = json_object_get(rootJ, "patchPath");
	if (patchPathJ)
		patchPath = json_string_value(patchPathJ);

	cableColors.clear();
	json_t* cableColorsJ = json_object_get(rootJ, "cableColors");
	if (cableColorsJ) {
		size_t i;
		json_t* cableColorJ;
		json_array_foreach(cableColorsJ, i, cableColorJ) {
			std::string colorStr = json_string_value(cableColorJ);
			cableColors.push_back(color::fromHexString(colorStr));
		}
	}

	moduleWhitelist.clear();
	json_t* moduleWhitelistJ = json_object_get(rootJ, "moduleWhitelist");
	if (moduleWhitelistJ) {
		const char* pluginSlug;
		json_t* moduleSlugsJ;
		json_object_foreach(moduleWhitelistJ, pluginSlug, moduleSlugsJ) {
			auto& moduleSlugs = moduleWhitelist[pluginSlug];
			size_t i;
			json_t* moduleSlugJ;
			json_array_foreach(moduleSlugsJ, i, moduleSlugJ) {
				std::string moduleSlug = json_string_value(moduleSlugJ);
				moduleSlugs.push_back(moduleSlug);
			}
		}
	}
}

void save(const std::string& path) {
	INFO("Saving settings %s", path.c_str());
	json_t* rootJ = toJson();
	if (!rootJ)
		return;

	FILE* file = fopen(path.c_str(), "w");
	if (!file)
		return;
	DEFER({
		fclose(file);
	});

	json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	json_decref(rootJ);
}

void load(const std::string& path) {
	INFO("Loading settings %s", path.c_str());
	FILE* file = fopen(path.c_str(), "r");
	if (!file)
		return;
	DEFER({
		fclose(file);
	});

	json_error_t error;
	json_t* rootJ = json_loadf(file, 0, &error);
	if (!rootJ)
		throw Exception(string::f("Settings file has invalid JSON at %d:%d %s", error.line, error.column, error.text));

	fromJson(rootJ);
	json_decref(rootJ);
}


} // namespace settings
} // namespace rack
