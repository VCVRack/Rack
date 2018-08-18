#include "../controller/1x8.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"

namespace rack_plugin_SynthKit {

struct M1x8Widget : ModuleWidget {
  M1x8Widget(M1x8Module *module);
};

M1x8Widget::M1x8Widget(M1x8Module *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/1x8.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(31, 366)));

  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 73), Port::INPUT, module,
                                          M1x8Module::TOP_INPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 158), Port::OUTPUT, module,
                                          M1x8Module::FIRST_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 179), Port::OUTPUT, module,
                                          M1x8Module::SECOND_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 200), Port::OUTPUT, module,
                                          M1x8Module::THIRD_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 221), Port::OUTPUT, module,
                                          M1x8Module::FOURTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 242), Port::OUTPUT, module,
                                          M1x8Module::FIFTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 263), Port::OUTPUT, module,
                                          M1x8Module::SIXTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 284), Port::OUTPUT, module,
                                          M1x8Module::SEVENTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 305), Port::OUTPUT, module,
                                          M1x8Module::EIGHTH_OUTPUT));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, M1x8) {
   Model *modelM1x8 = Model::create<M1x8Module, M1x8Widget>(
      "SynthKit", "1x8 Splitter", "1x8 Splitter", MIXER_TAG);
   return modelM1x8;
}
