#pragma once

#include "rack.hpp"
using namespace rack;

RACK_PLUGIN_DECLARE(JE);

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "JE"
#endif

