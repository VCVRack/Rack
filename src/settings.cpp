#include <jansson.h>

#include <settings.hpp>
#include <window.hpp>
#include <plugin.hpp>
#include <app/Scene.hpp>
#include <app/ModuleBrowser.hpp>
#include <engine/Engine.hpp>
#include <context.hpp>
#include <patch.hpp>


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
bool allowCursorLock = true;
KnobMode knobMode = KNOB_MODE_LINEAR;
float knobLinearSensitivity = 0.001f;
float sampleRate = 44100.0;
int threadCount = 1;
bool tooltips = true;
bool cpuMeter = false;
bool lockModules = false;
#if defined ARCH_MAC
	// Most Mac GPUs can't handle rendering the screen every frame, so use ~30 Hz by default.
	int frameSwapInterval = 2;
#else
	int frameSwapInterval = 1;
#endif
float autosaveInterval = 15.0;
bool skipLoadOnLaunch = false;
std::string patchPath;
std::list<std::string> recentPatchPaths;
std::vector<NVGcolor> cableColors = {
	color::fromHexString("#fc2d5aff"), // red
	color::fromHexString("#f9b130ff"), // orange
	// color::fromHexString("#f7da31ff"), // yellow
	color::fromHexString("#67c12dff"), // green
	color::fromHexString("#0f8df4ff"), // blue
	color::fromHexString("#8c1889ff"), // purple
};
std::map<std::string, std::set<std::string>> moduleWhitelist = {};
bool autoCheckUpdates = true;
bool showTipsOnLaunch = true;
int tipIndex = -1;


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

	json_object_set_new(rootJ, "allowCursorLock", json_boolean(allowCursorLock));

	json_object_set_new(rootJ, "knobMode", json_integer((int) knobMode));

	json_object_set_new(rootJ, "knobLinearSensitivity", json_real(knobLinearSensitivity));

	json_object_set_new(rootJ, "sampleRate", json_real(sampleRate));

	json_object_set_new(rootJ, "threadCount", json_integer(threadCount));

	json_object_set_new(rootJ, "tooltips", json_boolean(tooltips));

	json_object_set_new(rootJ, "cpuMeter", json_boolean(cpuMeter));

	json_object_set_new(rootJ, "lockModules", json_boolean(lockModules));

	json_object_set_new(rootJ, "frameSwapInterval", json_integer(frameSwapInterval));

	json_object_set_new(rootJ, "autosaveInterval", json_real(autosaveInterval));

	if (skipLoadOnLaunch)
		json_object_set_new(rootJ, "skipLoadOnLaunch", json_boolean(true));

	json_object_set_new(rootJ, "patchPath", json_string(patchPath.c_str()));

	json_t* recentPatchPathsJ = json_array();
	for (const std::string& path : recentPatchPaths) {
		json_array_append_new(recentPatchPathsJ, json_string(path.c_str()));
	}
	json_object_set_new(rootJ, "recentPatchPaths", recentPatchPathsJ);

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

	json_object_set_new(rootJ, "autoCheckUpdates", json_boolean(autoCheckUpdates));

	json_object_set_new(rootJ, "showTipsOnLaunch", json_boolean(showTipsOnLaunch));

	json_object_set_new(rootJ, "tipIndex", json_integer(tipIndex));

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

	json_t* allowCursorLockJ = json_object_get(rootJ, "allowCursorLock");
	if (allowCursorLockJ)
		allowCursorLock = json_boolean_value(allowCursorLockJ);

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

	json_t* tooltipsJ = json_object_get(rootJ, "tooltips");
	if (tooltipsJ)
		tooltips = json_boolean_value(tooltipsJ);

	json_t* cpuMeterJ = json_object_get(rootJ, "cpuMeter");
	if (cpuMeterJ)
		cpuMeter = json_boolean_value(cpuMeterJ);

	json_t* lockModulesJ = json_object_get(rootJ, "lockModules");
	if (lockModulesJ)
		lockModules = json_boolean_value(lockModulesJ);

	json_t* frameSwapIntervalJ = json_object_get(rootJ, "frameSwapInterval");
	if (frameSwapIntervalJ)
		frameSwapInterval = json_integer_value(frameSwapIntervalJ);

	json_t* autosaveIntervalJ = json_object_get(rootJ, "autosaveInterval");
	if (autosaveIntervalJ)
		autosaveInterval = json_number_value(autosaveIntervalJ);

	json_t* skipLoadOnLaunchJ = json_object_get(rootJ, "skipLoadOnLaunch");
	if (skipLoadOnLaunchJ)
		skipLoadOnLaunch = json_boolean_value(skipLoadOnLaunchJ);

	json_t* patchPathJ = json_object_get(rootJ, "patchPath");
	if (patchPathJ)
		patchPath = json_string_value(patchPathJ);

	recentPatchPaths.clear();
	json_t* recentPatchPathsJ = json_object_get(rootJ, "recentPatchPaths");
	if (recentPatchPathsJ) {
		size_t i;
		json_t* pathJ;
		json_array_foreach(recentPatchPathsJ, i, pathJ) {
			std::string path = json_string_value(pathJ);
			recentPatchPaths.push_back(path);
		}
	}

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
				moduleSlugs.insert(moduleSlug);
			}
		}
	}

	json_t* autoCheckUpdatesJ = json_object_get(rootJ, "autoCheckUpdates");
	if (autoCheckUpdatesJ)
		autoCheckUpdates = json_boolean_value(autoCheckUpdatesJ);

	json_t* showTipsOnLaunchJ = json_object_get(rootJ, "showTipsOnLaunch");
	if (showTipsOnLaunchJ)
		showTipsOnLaunch = json_boolean_value(showTipsOnLaunchJ);

	json_t* tipIndexJ = json_object_get(rootJ, "tipIndex");
	if (tipIndexJ)
		tipIndex = json_integer_value(tipIndexJ);
}

void save(const std::string& path) {
	INFO("Saving settings %s", path.c_str());
	json_t* rootJ = toJson();
	if (!rootJ)
		return;

	FILE* file = std::fopen(path.c_str(), "w");
	if (!file)
		return;
	DEFER({std::fclose(file);});

	json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	json_decref(rootJ);
}

void load(const std::string& path) {
	INFO("Loading settings %s", path.c_str());
	FILE* file = std::fopen(path.c_str(), "r");
	if (!file)
		return;
	DEFER({std::fclose(file);});

	json_error_t error;
	json_t* rootJ = json_loadf(file, 0, &error);
	if (!rootJ)
		throw Exception(string::f("Settings file has invalid JSON at %d:%d %s", error.line, error.column, error.text));

	fromJson(rootJ);
	json_decref(rootJ);
}


} // namespace settings
} // namespace rack
