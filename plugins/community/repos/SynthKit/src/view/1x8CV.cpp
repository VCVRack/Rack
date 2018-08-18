#include "../controller/1x8CV.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"

namespace rack_plugin_SynthKit {

struct M1x8CVWidget : ModuleWidget {
  M1x8CVWidget(M1x8CVModule *module);
};

M1x8CVWidget::M1x8CVWidget(M1x8CVModule *module) : ModuleWidget(module) {
  box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/1x8CV.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(46, 366)));

  addInput(Port::create<RCJackSmallLight>(Vec(17.73, 73), Port::INPUT, module,
                                          M1x8CVModule::TOP_INPUT));

  addInput(Port::create<RCJackSmallLight>(Vec(0.48, 158), Port::INPUT, module,
                                          M1x8CVModule::FIRST_CV));
  addOutput(Port::create<RCJackSmallDark>(Vec(25.23, 158), Port::OUTPUT, module,
                                          M1x8CVModule::FIRST_OUTPUT));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(51.5, 167.06), module, M1x8CVModule::FIRST_LED));

  addInput(Port::create<RCJackSmallLight>(Vec(0.48, 179), Port::INPUT, module,
                                          M1x8CVModule::SECOND_CV));
  addOutput(Port::create<RCJackSmallDark>(Vec(25.23, 179), Port::OUTPUT, module,
                                          M1x8CVModule::SECOND_OUTPUT));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(51.5, 188.06), module, M1x8CVModule::SECOND_LED));

  addInput(Port::create<RCJackSmallLight>(Vec(0.48, 200), Port::INPUT, module,
                                          M1x8CVModule::THIRD_CV));
  addOutput(Port::create<RCJackSmallDark>(Vec(25.23, 200), Port::OUTPUT, module,
                                          M1x8CVModule::THIRD_OUTPUT));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(51.5, 209.06), module, M1x8CVModule::THIRD_LED));

  addInput(Port::create<RCJackSmallLight>(Vec(0.48, 221), Port::INPUT, module,
                                          M1x8CVModule::FOURTH_CV));
  addOutput(Port::create<RCJackSmallDark>(Vec(25.23, 221), Port::OUTPUT, module,
                                          M1x8CVModule::FOURTH_OUTPUT));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(51.5, 230.06), module, M1x8CVModule::FOURTH_LED));

  addInput(Port::create<RCJackSmallLight>(Vec(0.48, 242), Port::INPUT, module,
                                          M1x8CVModule::FIFTH_CV));
  addOutput(Port::create<RCJackSmallDark>(Vec(25.23, 242), Port::OUTPUT, module,
                                          M1x8CVModule::FIFTH_OUTPUT));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(51.5, 251.06), module, M1x8CVModule::FIFTH_LED));

  addInput(Port::create<RCJackSmallLight>(Vec(0.48, 263), Port::INPUT, module,
                                          M1x8CVModule::SIXTH_CV));
  addOutput(Port::create<RCJackSmallDark>(Vec(25.23, 263), Port::OUTPUT, module,
                                          M1x8CVModule::SIXTH_OUTPUT));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(51.5, 272.06), module, M1x8CVModule::SIXTH_LED));

  addInput(Port::create<RCJackSmallLight>(Vec(0.48, 284), Port::INPUT, module,
                                          M1x8CVModule::SEVENTH_CV));
  addOutput(Port::create<RCJackSmallDark>(Vec(25.23, 284), Port::OUTPUT, module,
                                          M1x8CVModule::SEVENTH_OUTPUT));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(51.5, 293.06), module, M1x8CVModule::SEVENTH_LED));

  addInput(Port::create<RCJackSmallLight>(Vec(0.48, 305), Port::INPUT, module,
                                          M1x8CVModule::EIGHTH_CV));
  addOutput(Port::create<RCJackSmallDark>(Vec(25.23, 305), Port::OUTPUT, module,
                                          M1x8CVModule::EIGHTH_OUTPUT));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(51.5, 314.06), module, M1x8CVModule::EIGHTH_LED));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, M1x8CV) {
   Model *modelM1x8CV = Model::create<M1x8CVModule, M1x8CVWidget>(
      "SynthKit", "1x8 Splitter (CV)", "1x8 Splitter (CV)", MIXER_TAG);
   return modelM1x8CV;
}
