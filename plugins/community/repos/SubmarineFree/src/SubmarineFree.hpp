#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(SubmarineFree);

#ifdef USE_VST2
#define plugin "SubmarineFree"
#endif // USE_VST2

#include "ComponentLibrary/components.hpp"
