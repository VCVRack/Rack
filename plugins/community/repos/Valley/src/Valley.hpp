#include "rack.hpp"
using namespace rack;

#include <iomanip> // setprecision
#include <sstream> // stringstream
#define VALLEY_NAME "ValleyDev"

RACK_PLUGIN_DECLARE(Valley);

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "Valley"
#endif // USE_VST2
