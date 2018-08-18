#include "../controller/RotatingClockDivider2.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"

namespace rack_plugin_SynthKit {

struct RotatingClockDivider2Widget : ModuleWidget {
  RotatingClockDivider2Widget(RotatingClockDivider2Module *module);
};

RotatingClockDivider2Widget::RotatingClockDivider2Widget(
    RotatingClockDivider2Module *module)
    : ModuleWidget(module) {
  box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(
        SVG::load(assetPlugin(plugin, "res/RotatingClockDivider2.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(46, 366)));

  addInput(Port::create<RCJackSmallLight>(
      Vec(31.23, 109), Port::INPUT, module,
      RotatingClockDivider2Module::ROTATE_INPUT));
  addInput(
      Port::create<RCJackSmallLight>(Vec(31.23, 65), Port::INPUT, module,
                                     RotatingClockDivider2Module::TOP_INPUT));
  addInput(
      Port::create<RCJackSmallLight>(Vec(3.8, 87), Port::INPUT, module,
                                     RotatingClockDivider2Module::RESET_INPUT));

  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 158), Port::OUTPUT, module,
                                    RotatingClockDivider2Module::FIRST_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(
      Vec(10.23, 179), Port::OUTPUT, module,
      RotatingClockDivider2Module::SECOND_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 200), Port::OUTPUT, module,
                                    RotatingClockDivider2Module::THIRD_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(
      Vec(10.23, 221), Port::OUTPUT, module,
      RotatingClockDivider2Module::FOURTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 242), Port::OUTPUT, module,
                                    RotatingClockDivider2Module::FIFTH_OUTPUT));
  addOutput(
      Port::create<RCJackSmallDark>(Vec(10.23, 263), Port::OUTPUT, module,
                                    RotatingClockDivider2Module::SIXTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(
      Vec(10.23, 284), Port::OUTPUT, module,
      RotatingClockDivider2Module::SEVENTH_OUTPUT));
  addOutput(Port::create<RCJackSmallDark>(
      Vec(10.23, 305), Port::OUTPUT, module,
      RotatingClockDivider2Module::EIGHTH_OUTPUT));

  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 167.06), module, RotatingClockDivider2Module::FIRST_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 188.06), module, RotatingClockDivider2Module::SECOND_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 209.06), module, RotatingClockDivider2Module::THIRD_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 230.06), module, RotatingClockDivider2Module::FOURTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 251.06), module, RotatingClockDivider2Module::FIFTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 272.06), module, RotatingClockDivider2Module::SIXTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 293.06), module, RotatingClockDivider2Module::SEVENTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 314.06), module, RotatingClockDivider2Module::EIGHTH_LED));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, RotatingClockDivider2) {
   Model *modelRotatingClockDivider2 =
      Model::create<RotatingClockDivider2Module, RotatingClockDivider2Widget>(
         "SynthKit", "Shifting Clock Divider CV", "Shifting Clock Divider CV",
         UTILITY_TAG, CLOCK_TAG);
   return modelRotatingClockDivider2;
}
