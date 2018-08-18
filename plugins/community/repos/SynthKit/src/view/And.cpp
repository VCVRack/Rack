#include "../controller/And.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"

namespace rack_plugin_SynthKit {

struct AndWidget : ModuleWidget {
  AndWidget(AndModule *module);
};

AndWidget::AndWidget(AndModule *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/And.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(31, 366)));

  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 73), Port::INPUT, module,
                                          AndModule::TOP1_INPUT));
  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 115), Port::INPUT, module,
                                          AndModule::TOP2_INPUT));

  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 158), Port::OUTPUT, module,
                                          AndModule::TOP_OUTPUT));

  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 220), Port::INPUT, module,
                                          AndModule::BOTTOM1_INPUT));
  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 262), Port::INPUT, module,
                                          AndModule::BOTTOM2_INPUT));

  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 305), Port::OUTPUT, module,
                                          AndModule::BOTTOM_OUTPUT));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, And) {
   Model *modelAnd =
      Model::create<AndModule, AndWidget>("SynthKit", "And", "And", MIXER_TAG);
   return modelAnd;
}
