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
/** Process-scope globals, most of which are persisted across launches */
namespace settings {


// Runtime state, not serialized.

/** Path to settings.json */
extern std::string settingsPath;
extern bool devMode;
extern bool headless;
extern bool isPlugin;

// Persistent state, serialized to settings.json.

/** vcvrack.com user token */
extern std::string token;
/** Whether the window is maximized */
extern bool windowMaximized;
/** Size of window in pixels */
extern math::Vec windowSize;
/** Position in window in pixels */
extern math::Vec windowPos;
/** Reverse the zoom scroll direction */
extern bool invertZoom;
/** Ratio between UI pixel and physical screen pixel.
0 for auto.
*/
extern float pixelRatio;
/** Opacity of cables in the range [0, 1] */
extern float cableOpacity;
/** Straightness of cables in the range [0, 1]. Unitless and arbitrary. */
extern float cableTension;
extern float rackBrightness;
extern float haloBrightness;
/** Allows rack to hide and lock the cursor position when dragging knobs etc. */
extern bool allowCursorLock;
enum KnobMode {
	KNOB_MODE_LINEAR,
	KNOB_MODE_SCALED_LINEAR,
	KNOB_MODE_ROTARY_ABSOLUTE,
	KNOB_MODE_ROTARY_RELATIVE,
};
extern KnobMode knobMode;
extern bool knobScroll;
extern float knobLinearSensitivity;
extern float knobScrollSensitivity;
extern float sampleRate;
extern int threadCount;
extern bool tooltips;
extern bool cpuMeter;
extern bool lockModules;
extern int frameSwapInterval;
extern float autosaveInterval;
extern bool skipLoadOnLaunch;
extern std::list<std::string> recentPatchPaths;
extern std::vector<NVGcolor> cableColors;
extern bool autoCheckUpdates;
extern bool showTipsOnLaunch;
extern int tipIndex;
extern bool discordUpdateActivity;
enum BrowserSort {
	BROWSER_SORT_UPDATED,
	BROWSER_SORT_LAST_USED,
	BROWSER_SORT_MOST_USED,
	BROWSER_SORT_BRAND,
	BROWSER_SORT_NAME,
	BROWSER_SORT_RANDOM,
};
extern BrowserSort browserSort;
extern float browserZoom;

struct ModuleInfo {
	bool enabled = true;
	bool favorite = false;
	int added = 0;
	double lastAdded = NAN;
};
/** pluginSlug -> (moduleSlug -> ModuleInfo) */
extern std::map<std::string, std::map<std::string, ModuleInfo>> moduleInfos;
/** Returns a ModuleInfo if exists for the given slugs.
*/
ModuleInfo* getModuleInfo(const std::string& pluginSlug, const std::string& moduleSlug);

/** The VCV JSON API returns the data structure
{pluginSlug: [moduleSlugs] or true}
where "true" represents that the user is subscribed to the plugin (all modules and future modules).
C++ isn't weakly typed, so we need the PluginWhitelist data structure to store this information.
*/
struct PluginWhitelist {
	bool subscribed = false;
	std::set<std::string> moduleSlugs;
};
extern std::map<std::string, PluginWhitelist> moduleWhitelist;

bool isModuleWhitelisted(const std::string& pluginSlug, const std::string& moduleSlug);

PRIVATE void init();
PRIVATE json_t* toJson();
PRIVATE void fromJson(json_t* rootJ);
PRIVATE void save(std::string path = "");
PRIVATE void load(std::string path = "");


} // namespace settings
} // namespace rack
