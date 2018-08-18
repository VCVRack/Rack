#include "../controller/PrimeClockDivider.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"

namespace rack_plugin_SynthKit {

struct PrimeClockDividerWidget : ModuleWidget {
  PrimeClockDividerWidget(PrimeClockDividerModule *module);
};

PrimeClockDividerWidget::PrimeClockDividerWidget(
    PrimeClockDividerModule *module)
    : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(
        SVG::load(assetPlugin(plugin, "res/PrimeClockDivider.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(31, 366)));

  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 73), Port::INPUT, module,
                                          PrimeClockDividerModule::TOP_INPUT));
  addInput(
      Port::create<RCJackSmallLight>(Vec(10.23, 115), Port::INPUT, module,
                                     PrimeClockDividerModule::RESET_INPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 158), Port::OUTPUT, module,
                                    PrimeClockDividerModule::FIRST_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 179), Port::OUTPUT, module,
                                    PrimeClockDividerModule::SECOND_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 200), Port::OUTPUT, module,
                                    PrimeClockDividerModule::THIRD_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 221), Port::OUTPUT, module,
                                    PrimeClockDividerModule::FOURTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 242), Port::OUTPUT, module,
                                    PrimeClockDividerModule::FIFTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 263), Port::OUTPUT, module,
                                    PrimeClockDividerModule::SIXTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 284), Port::OUTPUT, module,
                                    PrimeClockDividerModule::SEVENTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 305), Port::OUTPUT, module,
                                    PrimeClockDividerModule::EIGHTH_OUTPUT));

  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 167.06), module, PrimeClockDividerModule::FIRST_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 188.06), module, PrimeClockDividerModule::SECOND_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 209.06), module, PrimeClockDividerModule::THIRD_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 230.06), module, PrimeClockDividerModule::FOURTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 251.06), module, PrimeClockDividerModule::FIFTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 272.06), module, PrimeClockDividerModule::SIXTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 293.06), module, PrimeClockDividerModule::SEVENTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 314.06), module, PrimeClockDividerModule::EIGHTH_LED));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, PrimeClockDivider) {
   Model *modelPrimeClockDivider =
      Model::create<PrimeClockDividerModule, PrimeClockDividerWidget>(
         "SynthKit", "Prime Clock Divider", "Prime Clock Divider", UTILITY_TAG,
         CLOCK_TAG);
   return modelPrimeClockDivider;
}
