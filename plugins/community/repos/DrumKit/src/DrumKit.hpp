#pragma once

#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(DrumKit);

#ifdef USE_VST2
#define plugin "DrumKit"
#endif // USE_VST2
