#include "rack.hpp"

using namespace rack;

namespace rack_plugin_ZZC {

struct ZZC_SmallSlider : SVGSlider {
  ZZC_SmallSlider() {
    Vec margin = Vec(3.5, 3.5);
    maxHandlePos = Vec(2, 2).plus(margin);
    minHandlePos = Vec(2, 24).plus(margin);
    setSVGs(SVG::load(assetPlugin(plugin, "res/sliders/ZZC-Small-Slider_BG.svg")), SVG::load(assetPlugin(plugin, "res/sliders/ZZC-Small-Slider_Handle.svg")));
    background->box.pos = margin;
    box.size = background->box.size.plus(margin.mult(2));
  }
};

} // namespace rack_plugin_ZZC
