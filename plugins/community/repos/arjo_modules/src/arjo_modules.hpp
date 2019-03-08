#include "rack.hpp"

using namespace rack;

namespace rack_plugin_arjo_modules {
}

RACK_PLUGIN_DECLARE(arjo_modules);

#ifdef USE_VST2
#define plugin "arjo_modules"
#endif // USE_VST2

namespace rack_plugin_arjo_modules {

struct small_port : SVGPort {
	small_port() {
		setSVG(SVG::load(assetPlugin(plugin,"res/small_port.svg")));
	}
};

} // namespace rack_plugin_arjo_modules
