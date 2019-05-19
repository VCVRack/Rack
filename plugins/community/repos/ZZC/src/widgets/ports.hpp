#include "rack.hpp"

using namespace rack;

namespace rack_plugin_ZZC {

struct ZZC_PJ_In_Port : SVGPort {
  ZZC_PJ_In_Port() {
    setSVG(SVG::load(assetPlugin(plugin, "res/sockets/ZZC-PJ-In.svg")));
  }
};

struct ZZC_PJ_Out_Port : SVGPort {
  ZZC_PJ_Out_Port() {
    setSVG(SVG::load(assetPlugin(plugin, "res/sockets/ZZC-PJ-Out.svg")));
  }
};

struct ZZC_PJ_Port : SVGPort {
  ZZC_PJ_Port() {
    setSVG(SVG::load(assetPlugin(plugin, "res/sockets/ZZC-PJ.svg")));
    shadow->box.size = Vec(29, 29);
    shadow->box.pos = Vec(-2, 0);
    shadow->blurRadius = 15.0f;
    shadow->opacity = 1.0f;
  }
};

} // namespace rack_plugin_ZZC
