#include "rack.hpp"

using namespace rack;

namespace rack_plugin_Edge {
}

RACK_PLUGIN_DECLARE(Edge);

#if defined(USE_VST2) && !defined(RACK_PLUGIN_SHARED)
#define plugin "Edge"
#endif // USE_VST2

namespace rack_plugin_Edge {

const NVGcolor BLUE = nvgRGBA(42, 87, 117, 255);
const NVGcolor RED = nvgRGBA(205, 31, 0, 255);
const NVGcolor YELLOW = nvgRGBA(255, 233, 0, 255);

} // namespace rack_plugin_Edge
