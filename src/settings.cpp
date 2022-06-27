#include <jansson.h>

#include <settings.hpp>
#include <window/Window.hpp>
#include <plugin.hpp>
#include <app/Scene.hpp>
#include <engine/Engine.hpp>
#include <context.hpp>
#include <patch.hpp>
#include <asset.hpp>


namespace rack {
namespace settings {


std::string settingsPath;
bool devMode = false;
bool headless = false;
bool isPlugin = false;

bool safeMode = false;
std::string token;
bool windowMaximized = false;
math::Vec windowSize = math::Vec(1024, 720);
math::Vec windowPos = math::Vec(NAN, NAN);
bool invertZoom = false;
float pixelRatio = 0.0;
float cableOpacity = 0.5;
float cableTension = 1.0;
float rackBrightness = 1.0;
float haloBrightness = 0.25;
bool allowCursorLock = true;
KnobMode knobMode = KNOB_MODE_LINEAR;
bool knobScroll = false;
float knobLinearSensitivity = 0.001f;
float knobScrollSensitivity = 0.001f;
float sampleRate = 0;
int threadCount = 1;
bool tooltips = true;
bool cpuMeter = false;
bool lockModules = false;
bool squeezeModules = true;
#if defined ARCH_MAC
	// Most Mac GPUs can't handle rendering the screen every frame, so use ~30 Hz by default.
	int frameSwapInterval = 2;
#else
	int frameSwapInterval = 1;
#endif
float autosaveInterval = 15.0;
bool skipLoadOnLaunch = false;
std::list<std::string> recentPatchPaths;
std::vector<NVGcolor> cableColors = {
	color::fromHexString("#f3374b"), // red
	color::fromHexString("#ffb437"), // yellow
	color::fromHexString("#00b56e"), // green
	color::fromHexString("#3695ef"), // blue
	color::fromHexString("#8b4ade"), // purple
};
bool autoCheckUpdates = true;
bool showTipsOnLaunch = true;
int tipIndex = -1;
BrowserSort browserSort = BROWSER_SORT_UPDATED;
float browserZoom = -1.f;
json_t* pluginSettingsJ = NULL;
std::map<std::string, std::map<std::string, ModuleInfo>> moduleInfos;
std::map<std::string, PluginWhitelist> moduleWhitelist;


ModuleInfo* getModuleInfo(const std::string& pluginSlug, const std::string& moduleSlug) {
	auto pluginIt = moduleInfos.find(pluginSlug);
	if (pluginIt == moduleInfos.end())
		return NULL;
	auto moduleIt = pluginIt->second.find(moduleSlug);
	if (moduleIt == pluginIt->second.end())
		return NULL;
	return &moduleIt->second;
}


bool isModuleWhitelisted(const std::string& pluginSlug, const std::string& moduleSlug) {
	auto pluginIt = moduleWhitelist.find(pluginSlug);
	// All modules in a plugin are visible if plugin set is empty.
	if (pluginIt == moduleWhitelist.end())
		return true;
	// All modules in a plugin are visible if plugin set is subscribed.
	const PluginWhitelist& plugin = pluginIt->second;
	if (plugin.subscribed)
		return true;
	// Check if plugin whitelist contains module
	auto moduleIt = plugin.moduleSlugs.find(moduleSlug);
	if (moduleIt == plugin.moduleSlugs.end())
		return false;
	return true;
}


void init() {
	settingsPath = asset::user("settings.json");
}


void destroy() {
	if (pluginSettingsJ)
		json_decref(pluginSettingsJ);
}


json_t* toJson() {
	json_t* rootJ = json_object();

	// Always disable safe mode when settings are saved.
	json_object_set_new(rootJ, "safeMode", json_boolean(false));

	json_object_set_new(rootJ, "token", json_string(token.c_str()));

	json_object_set_new(rootJ, "windowMaximized", json_boolean(windowMaximized));

	json_t* windowSizeJ = json_pack("[f, f]", windowSize.x, windowSize.y);
	json_object_set_new(rootJ, "windowSize", windowSizeJ);

	json_t* windowPosJ = json_pack("[f, f]", windowPos.x, windowPos.y);
	json_object_set_new(rootJ, "windowPos", windowPosJ);

	json_object_set_new(rootJ, "invertZoom", json_boolean(invertZoom));

	json_object_set_new(rootJ, "pixelRatio", json_real(pixelRatio));

	json_object_set_new(rootJ, "cableOpacity", json_real(cableOpacity));

	json_object_set_new(rootJ, "cableTension", json_real(cableTension));

	json_object_set_new(rootJ, "rackBrightness", json_real(rackBrightness));

	json_object_set_new(rootJ, "haloBrightness", json_real(haloBrightness));

	json_object_set_new(rootJ, "allowCursorLock", json_boolean(allowCursorLock));

	json_object_set_new(rootJ, "knobMode", json_integer((int) knobMode));

	json_object_set_new(rootJ, "knobScroll", json_boolean(knobScroll));

	json_object_set_new(rootJ, "knobLinearSensitivity", json_real(knobLinearSensitivity));

	json_object_set_new(rootJ, "knobScrollSensitivity", json_real(knobScrollSensitivity));

	json_object_set_new(rootJ, "sampleRate", json_real(sampleRate));

	json_object_set_new(rootJ, "threadCount", json_integer(threadCount));

	json_object_set_new(rootJ, "tooltips", json_boolean(tooltips));

	json_object_set_new(rootJ, "cpuMeter", json_boolean(cpuMeter));

	json_object_set_new(rootJ, "lockModules", json_boolean(lockModules));

	json_object_set_new(rootJ, "squeezeModules", json_boolean(squeezeModules));

	json_object_set_new(rootJ, "frameSwapInterval", json_integer(frameSwapInterval));

	json_object_set_new(rootJ, "autosaveInterval", json_real(autosaveInterval));

	if (skipLoadOnLaunch)
		json_object_set_new(rootJ, "skipLoadOnLaunch", json_boolean(true));

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

	json_object_set_new(rootJ, "autoCheckUpdates", json_boolean(autoCheckUpdates));

	json_object_set_new(rootJ, "showTipsOnLaunch", json_boolean(showTipsOnLaunch));

	json_object_set_new(rootJ, "tipIndex", json_integer(tipIndex));

	json_object_set_new(rootJ, "browserSort", json_integer((int) browserSort));

	json_object_set_new(rootJ, "browserZoom", json_real(browserZoom));

	// Merge pluginSettings instead of replace so plugins that fail to load don't cause their settings to be deleted.
	if (!pluginSettingsJ)
		pluginSettingsJ = json_object();
	plugin::settingsMergeJson(pluginSettingsJ);
	// Don't use *_set_new() here because we need to keep the reference to pluginSettingsJ.
	json_object_set(rootJ, "pluginSettings", pluginSettingsJ);

	// moduleInfos
	json_t* moduleInfosJ = json_object();
	for (const auto& pluginPair : moduleInfos) {
		json_t* pluginJ = json_object();
		for (const auto& modulePair : pluginPair.second) {
			const ModuleInfo& m = modulePair.second;
			json_t* moduleJ = json_object();
			{
				// To make setting.json smaller, only set properties if not default values.
				if (!m.enabled)
					json_object_set_new(moduleJ, "enabled", json_boolean(m.enabled));
				if (m.favorite)
					json_object_set_new(moduleJ, "favorite", json_boolean(m.favorite));
				if (m.added > 0)
					json_object_set_new(moduleJ, "added", json_integer(m.added));
				if (std::isfinite(m.lastAdded))
					json_object_set_new(moduleJ, "lastAdded", json_real(m.lastAdded));
			}
			if (json_object_size(moduleJ))
				json_object_set_new(pluginJ, modulePair.first.c_str(), moduleJ);
			else
				json_decref(moduleJ);
		}
		if (json_object_size(pluginJ))
			json_object_set_new(moduleInfosJ, pluginPair.first.c_str(), pluginJ);
		else
			json_decref(pluginJ);
	}
	json_object_set_new(rootJ, "moduleInfos", moduleInfosJ);

	// moduleWhitelist
	json_t* moduleWhitelistJ = json_object();
	for (const auto& pluginPair : moduleWhitelist) {
		const PluginWhitelist& plugin = pluginPair.second;
		json_t* pluginJ;

		// If plugin is subscribed, set to true, otherwise an array of module slugs.
		if (plugin.subscribed) {
			pluginJ = json_true();
		}
		else {
			pluginJ = json_array();
			for (const std::string& moduleSlug : plugin.moduleSlugs) {
				json_array_append_new(pluginJ, json_stringn(moduleSlug.c_str(), moduleSlug.size()));
			}
		}

		json_object_set_new(moduleWhitelistJ, pluginPair.first.c_str(), pluginJ);
	}
	json_object_set_new(rootJ, "moduleWhitelist", moduleWhitelistJ);

	return rootJ;
}

void fromJson(json_t* rootJ) {
	json_t* safeModeJ = json_object_get(rootJ, "safeMode");
	if (safeModeJ) {
		// If safe mode is enabled (e.g. by command line flag), don't disable it when loading.
		if (json_boolean_value(safeModeJ))
			safeMode = true;
	}

	json_t* tokenJ = json_object_get(rootJ, "token");
	if (tokenJ)
		token = json_string_value(tokenJ);

	json_t* windowMaximizedJ = json_object_get(rootJ, "windowMaximized");
	if (windowMaximizedJ)
		windowMaximized = json_boolean_value(windowMaximizedJ);

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

	json_t* invertZoomJ = json_object_get(rootJ, "invertZoom");
	if (invertZoomJ)
		invertZoom = json_boolean_value(invertZoomJ);

	json_t* pixelRatioJ = json_object_get(rootJ, "pixelRatio");
	if (pixelRatioJ)
		pixelRatio = json_number_value(pixelRatioJ);

	json_t* cableOpacityJ = json_object_get(rootJ, "cableOpacity");
	if (cableOpacityJ)
		cableOpacity = json_number_value(cableOpacityJ);

	json_t* cableTensionJ = json_object_get(rootJ, "cableTension");
	if (cableTensionJ)
		cableTension = json_number_value(cableTensionJ);

	json_t* rackBrightnessJ = json_object_get(rootJ, "rackBrightness");
	if (rackBrightnessJ)
		rackBrightness = json_number_value(rackBrightnessJ);

	json_t* haloBrightnessJ = json_object_get(rootJ, "haloBrightness");
	if (haloBrightnessJ)
		haloBrightness = json_number_value(haloBrightnessJ);

	json_t* allowCursorLockJ = json_object_get(rootJ, "allowCursorLock");
	if (allowCursorLockJ)
		allowCursorLock = json_boolean_value(allowCursorLockJ);

	json_t* knobModeJ = json_object_get(rootJ, "knobMode");
	if (knobModeJ)
		knobMode = (KnobMode) json_integer_value(knobModeJ);

	json_t* knobScrollJ = json_object_get(rootJ, "knobScroll");
	if (knobScrollJ)
		knobScroll = json_boolean_value(knobScrollJ);

	json_t* knobLinearSensitivityJ = json_object_get(rootJ, "knobLinearSensitivity");
	if (knobLinearSensitivityJ)
		knobLinearSensitivity = json_number_value(knobLinearSensitivityJ);

	json_t* knobScrollSensitivityJ = json_object_get(rootJ, "knobScrollSensitivity");
	if (knobScrollSensitivityJ)
		knobScrollSensitivity = json_number_value(knobScrollSensitivityJ);

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

	json_t* squeezeModulesJ = json_object_get(rootJ, "squeezeModules");
	if (squeezeModulesJ)
		squeezeModules = json_boolean_value(squeezeModulesJ);

	json_t* frameSwapIntervalJ = json_object_get(rootJ, "frameSwapInterval");
	if (frameSwapIntervalJ)
		frameSwapInterval = json_integer_value(frameSwapIntervalJ);

	json_t* autosaveIntervalJ = json_object_get(rootJ, "autosaveInterval");
	if (autosaveIntervalJ)
		autosaveInterval = json_number_value(autosaveIntervalJ);

	json_t* skipLoadOnLaunchJ = json_object_get(rootJ, "skipLoadOnLaunch");
	if (skipLoadOnLaunchJ)
		skipLoadOnLaunch = json_boolean_value(skipLoadOnLaunchJ);

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

	json_t* autoCheckUpdatesJ = json_object_get(rootJ, "autoCheckUpdates");
	if (autoCheckUpdatesJ)
		autoCheckUpdates = json_boolean_value(autoCheckUpdatesJ);

	json_t* showTipsOnLaunchJ = json_object_get(rootJ, "showTipsOnLaunch");
	if (showTipsOnLaunchJ)
		showTipsOnLaunch = json_boolean_value(showTipsOnLaunchJ);

	json_t* tipIndexJ = json_object_get(rootJ, "tipIndex");
	if (tipIndexJ)
		tipIndex = json_integer_value(tipIndexJ);

	json_t* browserSortJ = json_object_get(rootJ, "browserSort");
	if (browserSortJ)
		browserSort = (BrowserSort) json_integer_value(browserSortJ);

	json_t* browserZoomJ = json_object_get(rootJ, "browserZoom");
	if (browserZoomJ)
		browserZoom = json_number_value(browserZoomJ);

	// Delete previous pluginSettings object
	if (pluginSettingsJ) {
		json_decref(pluginSettingsJ);
		pluginSettingsJ = NULL;
	}
	pluginSettingsJ = json_object_get(rootJ, "pluginSettings");
	if (pluginSettingsJ)
		json_incref(pluginSettingsJ);

	moduleInfos.clear();
	json_t* moduleInfosJ = json_object_get(rootJ, "moduleInfos");
	if (moduleInfosJ) {
		const char* pluginSlug;
		json_t* pluginJ;
		json_object_foreach(moduleInfosJ, pluginSlug, pluginJ) {
			const char* moduleSlug;
			json_t* moduleJ;
			json_object_foreach(pluginJ, moduleSlug, moduleJ) {
				ModuleInfo m;

				json_t* enabledJ = json_object_get(moduleJ, "enabled");
				if (enabledJ)
					m.enabled = json_boolean_value(enabledJ);

				json_t* favoriteJ = json_object_get(moduleJ, "favorite");
				if (favoriteJ)
					m.favorite = json_boolean_value(favoriteJ);

				json_t* addedJ = json_object_get(moduleJ, "added");
				if (addedJ)
					m.added = json_integer_value(addedJ);

				json_t* lastAddedJ = json_object_get(moduleJ, "lastAdded");
				if (lastAddedJ)
					m.lastAdded = json_number_value(lastAddedJ);

				moduleInfos[pluginSlug][moduleSlug] = m;
			}
		}
	}

	moduleWhitelist.clear();
	json_t* moduleWhitelistJ = json_object_get(rootJ, "moduleWhitelist");
	if (moduleWhitelistJ) {
		const char* pluginSlug;
		json_t* pluginJ;
		json_object_foreach(moduleWhitelistJ, pluginSlug, pluginJ) {
			auto& plugin = moduleWhitelist[pluginSlug];

			if (json_is_true(pluginJ)) {
				plugin.subscribed = true;
				continue;
			}

			size_t moduleIndex;
			json_t* moduleJ;
			json_array_foreach(pluginJ, moduleIndex, moduleJ) {
				std::string moduleSlug = json_string_value(moduleJ);
				plugin.moduleSlugs.insert(moduleSlug);
			}
		}
	}
}

void save(std::string path) {
	if (path.empty())
		path = settingsPath;

	INFO("Saving settings %s", path.c_str());
	json_t* rootJ = toJson();
	if (!rootJ)
		return;

	FILE* file = std::fopen(path.c_str(), "w");
	if (!file)
		return;
	DEFER({std::fclose(file);});

	json_dumpf(rootJ, file, JSON_INDENT(2));
	json_decref(rootJ);
}

void load(std::string path) {
	if (path.empty())
		path = settingsPath;

	INFO("Loading settings %s", path.c_str());
	FILE* file = std::fopen(path.c_str(), "r");
	if (!file)
		return;
	DEFER({std::fclose(file);});

	json_error_t error;
	json_t* rootJ = json_loadf(file, 0, &error);
	if (!rootJ)
		throw Exception("Settings file has invalid JSON at %d:%d %s", error.line, error.column, error.text);

	fromJson(rootJ);
	json_decref(rootJ);
}


} // namespace settings
} // namespace rack
