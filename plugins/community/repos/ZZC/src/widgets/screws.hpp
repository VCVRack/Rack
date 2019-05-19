#include "rack.hpp"

using namespace rack;

namespace rack_plugin_ZZC {

struct ZZC_Screw : SVGScrew {
  ZZC_Screw() {
    sw->setSVG(SVG::load(assetPlugin(plugin, "res/screws/ZZC-Screw.svg")));
    box.size = sw->box.size;
  }
};

} // namespace rack_plugin_ZZC
