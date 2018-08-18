#include "../controller/Seq4.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"
#include "knobs.hpp"

namespace rack_plugin_SynthKit {

struct Seq4Widget : ModuleWidget {
  Seq4Widget(Seq4Module *module);
};

Seq4Widget::Seq4Widget(Seq4Module *module) : ModuleWidget(module) {
  box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Seq4.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(31, 366)));

  addInput(Port::create<RCJackSmallLight>(Vec(10.23, 73), Port::INPUT, module,
                                          Seq4Module::CLOCK_INPUT));

  addParam(ParamWidget::create<Knob30Snap>(
      Vec(7.5, 123), module, Seq4Module::OCTAVE_PARAM, 0.0, 8.0, 4.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(13, 185), module, Seq4Module::SEQ1_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(13, 211), module, Seq4Module::SEQ2_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(13, 237), module, Seq4Module::SEQ3_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(13, 263), module, Seq4Module::SEQ4_PARAM, 0.0, 11.0, 5.0));

  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 191.28), module, Seq4Module::FIRST_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 217.28), module, Seq4Module::SECOND_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 243.28), module, Seq4Module::THIRD_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(36.5, 269.28), module, Seq4Module::FOURTH_LED));

  addOutput(Port::create<RCJackSmallDark>(Vec(10.23, 305), Port::OUTPUT, module,
                                          Seq4Module::GATE_OUTPUT));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, Seq4) {
   Model *modelSeq4 = Model::create<Seq4Module, Seq4Widget>(
      "SynthKit", "4-Step Sequencer", "4-Step Sequencer", SEQUENCER_TAG);
   return modelSeq4;
}
