#define RACK_SKIP_RINGBUFFER
// #define RACK_SKIP_RESAMPLER
#include "rack.hpp"

using namespace rack;

RACK_PLUGIN_DECLARE(FrozenWasteland);

#ifdef USE_VST2
#define plugin "FrozenWasteland"
#endif // USE_VST2
