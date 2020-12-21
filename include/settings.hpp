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


// Runtime state, not serialized.

extern bool devMode;
extern bool headless;

// Persistent state, serialized to settings.json.

/** vcvrack.com user token */
extern std::string token;
/** Size of window in pixels */
extern math::Vec windowSize;
/** Position in window in pixels */
extern math::Vec windowPos;
/** Rack zoom level, log2. E.g. 100% = 0, 200% = 1, 50% = -1. */
extern float zoom;
/** Reverse the zoom scroll direction */
extern bool invertZoom;
/** Opacity of cables in the range [0, 1] */
extern float cableOpacity;
/** Straightness of cables in the range [0, 1]. Unitless and arbitrary. */
extern float cableTension;
/** Allows rack to hide and lock the cursor position when dragging knobs etc. */
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
extern bool tooltips;
extern bool cpuMeter;
extern bool lockModules;
extern int frameSwapInterval;
extern float autosaveInterval;
extern bool skipLoadOnLaunch;
extern std::string patchPath;
extern std::list<std::string> recentPatchPaths;
extern std::vector<NVGcolor> cableColors;
extern bool autoCheckUpdates;
extern bool showTipsOnLaunch;
extern int tipIndex;
enum ModuleBrowserSort {
	MODULE_BROWSER_SORT_UPDATED,
	MODULE_BROWSER_SORT_MOST_USED,
	MODULE_BROWSER_SORT_LAST_USED,
	MODULE_BROWSER_SORT_BRAND,
	MODULE_BROWSER_SORT_NAME,
	MODULE_BROWSER_SORT_RANDOM,
};
extern ModuleBrowserSort moduleBrowserSort;
extern float moduleBrowserZoom;
// pluginSlug -> moduleSlugs
extern std::map<std::string, std::set<std::string>> moduleWhitelist;

struct ModuleUsage {
	int count = 0;
	double lastTime = NAN;
};
extern std::map<std::string, std::map<std::string, ModuleUsage>> moduleUsages;

json_t* toJson();
void fromJson(json_t* rootJ);
void save(const std::string& path);
void load(const std::string& path);


} // namespace settings
} // namespace rack
