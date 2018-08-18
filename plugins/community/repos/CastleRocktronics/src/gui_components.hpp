#include "common.hpp"

namespace rack_plugin_CastleRocktronics {

struct ShadedKnob : virtual Knob, FramebufferWidget {
  float minAngle = -2.35619f;
  float maxAngle = 2.35619f;

  TransformWidget* markerTransformWidget;
  SVGWidget* markerSVGWidget;

  SVGWidget* highlightWidget;

  TransformWidget* knobTransformWidget;
  SVGWidget* knobSVGWidget;

  CircularShadow* shadowWidget;
  float shadowBlur = 2.0f;
  Vec shadowOffset = Vec(-2, 2);

  ShadedKnob();
  void setMarker(std::shared_ptr<SVG> svg);
  void setHighlight(std::shared_ptr<SVG> svg);
  void setKnob(std::shared_ptr<SVG> svg);

  void step() override;
  void onChange(EventChange& e) override;
};

struct TenHex : SVGPort {
  TenHex() {
    background->svg = SVG::load(
        assetPlugin(plugin, "res/components/ports/10mmHex_black.svg"));
    background->wrap();
    box.size = background->box.size;
  }
};

struct SevenHalfHex : SVGPort {
  SevenHalfHex() {
    background->svg =
        SVG::load(assetPlugin(plugin, "res/components/ports/7_5mm_hex.svg"));
    background->wrap();
    box.size = background->box.size;
  }
};

struct SevenHalfKnurled : SVGPort {
  SevenHalfKnurled() {
    background->svg =
        SVG::load(assetPlugin(plugin, "res/components/ports/knurled-jack.svg"));
    background->wrap();
    box.size = background->box.size;
  }
};

struct CastleTrimpot : ShadedKnob {
  CastleTrimpot() {
    shadowBlur = 1.0f;
    shadowOffset = Vec(-1, 1);

    setKnob(SVG::load(assetPlugin(plugin, "res/components/trimpot/shaft.svg")));
    setHighlight(
        SVG::load(assetPlugin(plugin, "res/components/trimpot/highlight.svg")));
    setMarker(
        SVG::load(assetPlugin(plugin, "res/components/trimpot/marker.svg")));
  }
};

struct DelvinToggleTwo : SVGSwitch, ToggleSwitch {
  DelvinToggleTwo() {
    addFrame(SVG::load(
        assetPlugin(plugin, "res/components/switches/delvin/2pos_0.svg")));
    addFrame(SVG::load(
        assetPlugin(plugin, "res/components/switches/delvin/2pos_1.svg")));
  }
};

struct MThreeScrew : SVGScrew {
  MThreeScrew() {
    sw->setSVG(
        SVG::load(assetPlugin(plugin, "res/components/screws/screw.svg")));
    box.size = sw->box.size;
  }
};

} // namespace rack_plugin_CastleRocktronics
