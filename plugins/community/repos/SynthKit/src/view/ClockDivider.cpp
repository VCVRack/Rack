#include "../controller/ClockDivider.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"

namespace rack_plugin_SynthKit {

struct ClockDividerWidget : ModuleWidget {
  ClockDividerWidget(ClockDividerModule *module);
};

ClockDividerWidget::ClockDividerWidget(ClockDividerModule *module)
    : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(
        SVG::load(assetPlugin(plugin, "res/ClockDivider.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(31, 366)));

  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 73), Port::INPUT, module,
                                          ClockDividerModule::TOP_INPUT));
  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 115), Port::INPUT, module,
                                          ClockDividerModule::RESET_INPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 158), Port::OUTPUT, module,
                                          ClockDividerModule::FIRST_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 179), Port::OUTPUT, module,
                                          ClockDividerModule::SECOND_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 200), Port::OUTPUT, module,
                                          ClockDividerModule::THIRD_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 221), Port::OUTPUT, module,
                                          ClockDividerModule::FOURTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 242), Port::OUTPUT, module,
                                          ClockDividerModule::FIFTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 263), Port::OUTPUT, module,
                                          ClockDividerModule::SIXTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 284), Port::OUTPUT, module,
                                          ClockDividerModule::SEVENTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 305), Port::OUTPUT, module,
                                          ClockDividerModule::EIGHTH_OUTPUT));

  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 167.06), module, ClockDividerModule::FIRST_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 188.06), module, ClockDividerModule::SECOND_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 209.06), module, ClockDividerModule::THIRD_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 230.06), module, ClockDividerModule::FOURTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 251.06), module, ClockDividerModule::FIFTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 272.06), module, ClockDividerModule::SIXTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 293.06), module, ClockDividerModule::SEVENTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 314.06), module, ClockDividerModule::EIGHTH_LED));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, ClockDivider) {
   Model *modelClockDivider =
      Model::create<ClockDividerModule, ClockDividerWidget>(
         "SynthKit", "Clock Divider", "Clock Divider", UTILITY_TAG, CLOCK_TAG);
   return modelClockDivider;
}

