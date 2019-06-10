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
extern bool allowCursorLock;
extern bool realTime;
extern float sampleRate;
extern int threadCount;
extern bool paramTooltip;
extern bool cpuMeter;
extern bool lockModules;
extern float frameRateLimit;
extern bool frameRateSync;
extern float autosavePeriod;
extern bool skipLoadOnLaunch;
extern std::string patchPath;
// (plugin, model) -> score
extern std::map<std::tuple<std::string, std::string>, float> favoriteScores;
extern std::vector<NVGcolor> cableColors;

json_t *toJson();
void fromJson(json_t *rootJ);
void save(const std::string &path);
void load(const std::string &path);


} // namespace settings
} // namespace rack
