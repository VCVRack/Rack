#pragma once
#include <vector>
#include <set>
#include <map>
#include <list>
#include <tuple>

#include <jansson.h>

#include <common.hpp>
#include <math.hpp>
#include <color.hpp>


namespace rack {


/** Process-level globals. */
namespace settings {


/** Runtime state, not serialized. */
extern bool devMode;
extern bool headless;

/** Persistent state, serialized to settings.json. */
extern std::string token;
extern math::Vec windowSize;
extern math::Vec windowPos;
extern float zoom;
extern bool invertZoom;
extern float cableOpacity;
extern float cableTension;
extern bool allowCursorLock;
enum KnobMode {
	KNOB_MODE_LINEAR,
	KNOB_MODE_SCALED_LINEAR,
	KNOB_MODE_ROTARY_ABSOLUTE,
	KNOB_MODE_ROTARY_RELATIVE,
};
extern KnobMode knobMode;
extern float knobLinearSensitivity;
extern float sampleRate;
extern int threadCount;
extern bool paramTooltip;
extern bool cpuMeter;
extern bool lockModules;
extern int frameSwapInterval;
extern float autosavePeriod;
extern bool skipLoadOnLaunch;
extern std::string patchPath;
extern std::list<std::string> recentPatchPaths;
extern std::vector<NVGcolor> cableColors;
// pluginSlug -> moduleSlugs
extern std::map<std::string, std::set<std::string>> moduleWhitelist;
extern bool checkAppUpdates;

json_t* toJson();
void fromJson(json_t* rootJ);
void save(const std::string& path);
void load(const std::string& path);


} // namespace settings
} // namespace rack
