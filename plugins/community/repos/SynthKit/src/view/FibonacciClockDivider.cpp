#include "../controller/FibonacciClockDivider.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"

namespace rack_plugin_SynthKit {

struct FibonacciClockDividerWidget : ModuleWidget {
  FibonacciClockDividerWidget(FibonacciClockDividerModule *module);
};

FibonacciClockDividerWidget::FibonacciClockDividerWidget(
    FibonacciClockDividerModule *module)
    : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(
        SVG::load(assetPlugin(plugin, "res/FibonacciClockDivider.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(31, 366)));

  addInput(
      Port::create<RCJackSmallLight>(Vec(10.23, 73), Port::INPUT, module,
                                     FibonacciClockDividerModule::TOP_INPUT));
  addInput(
      Port::create<RCJackSmallLight>(Vec(10.23, 115), Port::INPUT, module,
                                     FibonacciClockDividerModule::RESET_INPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 158), Port::OUTPUT, module,
                                    FibonacciClockDividerModule::FIRST_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(
      Vec(10.23, 179), Port::OUTPUT, module,
      FibonacciClockDividerModule::SECOND_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 200), Port::OUTPUT, module,
                                    FibonacciClockDividerModule::THIRD_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(
      Vec(10.23, 221), Port::OUTPUT, module,
      FibonacciClockDividerModule::FOURTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 242), Port::OUTPUT, module,
                                    FibonacciClockDividerModule::FIFTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 263), Port::OUTPUT, module,
                                    FibonacciClockDividerModule::SIXTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(
      Vec(10.23, 284), Port::OUTPUT, module,
      FibonacciClockDividerModule::SEVENTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(
      Vec(10.23, 305), Port::OUTPUT, module,
      FibonacciClockDividerModule::EIGHTH_OUTPUT));

  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 167.06), module, FibonacciClockDividerModule::FIRST_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 188.06), module, FibonacciClockDividerModule::SECOND_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 209.06), module, FibonacciClockDividerModule::THIRD_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 230.06), module, FibonacciClockDividerModule::FOURTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 251.06), module, FibonacciClockDividerModule::FIFTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 272.06), module, FibonacciClockDividerModule::SIXTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 293.06), module, FibonacciClockDividerModule::SEVENTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 314.06), module, FibonacciClockDividerModule::EIGHTH_LED));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, FibonacciClockDivider) {
   Model *modelFibonacciClockDivider =
      Model::create<FibonacciClockDividerModule, FibonacciClockDividerWidget>(
         "SynthKit", "Fibonacci Clock Divider", "Fibonacci Clock Divider",
         UTILITY_TAG, CLOCK_TAG);
   return modelFibonacciClockDivider;
}
