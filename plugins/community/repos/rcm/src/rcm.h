#include "rack.hpp"

using namespace rack;

namespace rack_plugin_rcm {
   namespace SongRoll {
   }
}

RACK_PLUGIN_DECLARE(rcm);

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "rcm"
#endif // USE_VST2

