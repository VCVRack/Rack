#pragma once
#include "common.hpp"
#include "math.hpp"
#include "plugin/Model.hpp"
#include <jansson.h>


namespace rack {


struct Settings {
	std::string token;
	math::Vec windowSize;
	math::Vec windowPos;
	float zoom = 1.0;
	bool invertZoom = false;
	float cableOpacity = 0.5;
	float cableTension = 0.5;
	bool allowCursorLock = true;
	float sampleRate = 44100.0;
	int threadCount = 1;
	bool paramTooltip = false;
	bool cpuMeter = false;
	bool lockModules = false;
	bool checkVersion = true;
	float frameRateLimit = 70.0;
	bool frameRateSync = true;
	bool skipLoadOnLaunch = false;
	std::string patchPath;
	std::set<plugin::Model*> favoriteModels;

	json_t *toJson();
	void fromJson(json_t *rootJ);
	void save(std::string filename);
	void load(std::string filename);
};


extern Settings settings;


} // namespace rack
