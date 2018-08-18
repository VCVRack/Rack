#include "../controller/RotatingClockDivider.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"

namespace rack_plugin_SynthKit {

struct RotatingClockDividerWidget : ModuleWidget {
  RotatingClockDividerWidget(RotatingClockDividerModule *module);
};

RotatingClockDividerWidget::RotatingClockDividerWidget(
    RotatingClockDividerModule *module)
    : ModuleWidget(module) {
  box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(
        SVG::load(assetPlugin(plugin, "res/RotatingClockDivider.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(46, 366)));

  addInput(
      Port::create<RCJackSmallLight>(Vec(31.23, 109), Port::INPUT, module,
                                     RotatingClockDividerModule::ROTATE_INPUT));
  addInput(
      Port::create<RCJackSmallLight>(Vec(31.23, 65), Port::INPUT, module,
                                     RotatingClockDividerModule::TOP_INPUT));
  addInput(
      Port::create<RCJackSmallLight>(Vec(3.8, 87), Port::INPUT, module,
                                     RotatingClockDividerModule::RESET_INPUT));

  addOutput(
      Port::create<RCJackSmallDark>(Vec(17.23, 158), Port::OUTPUT, module,
                                    RotatingClockDividerModule::FIRST_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(17.23, 179), Port::OUTPUT, module,
                                    RotatingClockDividerModule::SECOND_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(17.23, 200), Port::OUTPUT, module,
                                    RotatingClockDividerModule::THIRD_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(17.23, 221), Port::OUTPUT, module,
                                    RotatingClockDividerModule::FOURTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(17.23, 242), Port::OUTPUT, module,
                                    RotatingClockDividerModule::FIFTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(17.23, 263), Port::OUTPUT, module,
                                    RotatingClockDividerModule::SIXTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(
      Vec(17.23, 284), Port::OUTPUT, module,
      RotatingClockDividerModule::SEVENTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(17.23, 305), Port::OUTPUT, module,
                                    RotatingClockDividerModule::EIGHTH_OUTPUT));

  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(44, 167.06), module, RotatingClockDividerModule::FIRST_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(44, 188.06), module, RotatingClockDividerModule::SECOND_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(44, 209.06), module, RotatingClockDividerModule::THIRD_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(44, 230.06), module, RotatingClockDividerModule::FOURTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(44, 251.06), module, RotatingClockDividerModule::FIFTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(44, 272.06), module, RotatingClockDividerModule::SIXTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(44, 293.06), module, RotatingClockDividerModule::SEVENTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(44, 314.06), module, RotatingClockDividerModule::EIGHTH_LED));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, RotatingClockDivider) {
   Model *modelRotatingClockDivider =
      Model::create<RotatingClockDividerModule, RotatingClockDividerWidget>(
         "SynthKit", "Rotating Clock Divider", "Rotating Clock Divider",
         UTILITY_TAG, CLOCK_TAG);
   return modelRotatingClockDivider;
}
