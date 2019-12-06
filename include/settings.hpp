#pragma once
#include <common.hpp>
#include <math.hpp>
#include <color.hpp>
#include <vector>
#include <map>
#include <tuple>
#include <jansson.h>


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
enum KnobMode {
	KNOB_MODE_LINEAR_LOCKED = 0,
	KNOB_MODE_LINEAR,
	KNOB_MODE_SCALED_LINEAR_LOCKED = 100,
	KNOB_MODE_SCALED_LINEAR,
	KNOB_MODE_ROTARY_ABSOLUTE = 200,
	KNOB_MODE_ROTARY_RELATIVE,
};
extern KnobMode knobMode;
extern float sampleRate;
extern int threadCount;
extern bool paramTooltip;
extern bool cpuMeter;
extern bool lockModules;
extern int frameSwapInterval;
extern float autosavePeriod;
extern bool skipLoadOnLaunch;
extern std::string patchPath;
extern std::vector<NVGcolor> cableColors;
// pluginSlug -> moduleSlugs
extern std::map<std::string, std::vector<std::string>> moduleWhitelist;

json_t* toJson();
void fromJson(json_t* rootJ);
void save(const std::string& path);
void load(const std::string& path);


} // namespace settings
} // namespace rack
