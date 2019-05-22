#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(EH_modules);

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "EH_modules"
#endif // USE_VST2
