#include "rack.hpp"

#pragma once

#include "HetrickUtilities.hpp"

using namespace rack;

namespace rack_plugin_HetrickCV {
}


RACK_PLUGIN_DECLARE(HetrickCV);

#ifdef USE_VST2
#define plugin "HetrickCV"
#endif // USE_VST2
