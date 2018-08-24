#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(bsp);

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "bsp"
#endif // USE_VST2
