#ifndef INCLUDE_BACONPLUGS_HPP
#define INCLUDE_BACONPLUGS_HPP

#include "rack.hpp"

#include <map>
#include <vector>
#include <string>

using namespace rack;

#define SCREW_WIDTH 15
#define RACK_HEIGHT 380

RACK_PLUGIN_DECLARE(BaconMusic);

#ifdef USE_VST2
#define plugin "BaconMusic"
#endif // USE_VST2

#include "Components.hpp"

#endif
