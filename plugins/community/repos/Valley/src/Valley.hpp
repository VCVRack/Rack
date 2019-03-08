#include "rack.hpp"
using namespace rack;

#include <iomanip> // setprecision
#include <sstream> // stringstream
#define VALLEY_NAME "ValleyDev"
namespace rack_plugin_Valley {

RACK_PLUGIN_DECLARE(Valley);

#ifdef USE_VST2
#define plugin "Valley"
#endif // USE_VST2

}
