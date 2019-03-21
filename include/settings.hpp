#pragma once
#include "common.hpp"
#include "math.hpp"
#include "plugin/Model.hpp"
#include <jansson.h>


namespace rack {
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
extern float sampleRate;
extern int threadCount;
extern bool paramTooltip;
extern bool cpuMeter;
extern bool lockModules;
extern bool checkVersion;
extern float frameRateLimit;
extern bool frameRateSync;
extern bool skipLoadOnLaunch;
extern std::string patchPath;
extern std::set<plugin::Model*> favoriteModels;

json_t *toJson();
void fromJson(json_t *rootJ);
void save(const std::string &path);
void load(const std::string &path);


} // namespace settings
} // namespace rack
