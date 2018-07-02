#include "rack.hpp"


using namespace rack;


RACK_PLUGIN_DECLARE(Befaco);

#ifdef USE_VST2
#define plugin "Befaco"
#endif // USE_VST2


struct Knurlie : SVGScrew {
	Knurlie() {
		sw->svg = SVG::load(assetPlugin(plugin, "res/Knurlie.svg"));
		sw->wrap();
		box.size = sw->box.size;
	}
};

void springReverbInit();
