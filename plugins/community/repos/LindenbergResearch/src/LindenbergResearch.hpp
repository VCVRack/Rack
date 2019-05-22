#pragma once

#include <cmath>
#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(LindenbergResearch);

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "LindenbergResearch"
#endif // USE_VST2

#include "asset.hpp"
#include "widgets.hpp"
#include "LRComponents.hpp"
