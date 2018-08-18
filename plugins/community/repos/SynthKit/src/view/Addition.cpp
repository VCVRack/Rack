#include "../controller/Addition.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"

namespace rack_plugin_SynthKit {

struct AdditionWidget : ModuleWidget {
  AdditionWidget(AdditionModule *module);
};

AdditionWidget::AdditionWidget(AdditionModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Addition.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(31, 366)));

  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 73), Port::INPUT, module,
                                          AdditionModule::TOP1_INPUT));
  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 115), Port::INPUT, module,
                                         AdditionModule::TOP2_INPUT));

  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 158), Port::OUTPUT, module,
                                          AdditionModule::TOP_OUTPUT));

  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 220), Port::INPUT, module,
                                          AdditionModule::BOTTOM1_INPUT));
  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 262), Port::INPUT, module,
                                         AdditionModule::BOTTOM2_INPUT));

  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 305), Port::OUTPUT, module,
                                          AdditionModule::BOTTOM_OUTPUT));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, Addition) {
   Model *modelAddition = Model::create<AdditionModule, AdditionWidget>(
      "SynthKit", "Addition", "Addition", MIXER_TAG);
   return modelAddition;
}
