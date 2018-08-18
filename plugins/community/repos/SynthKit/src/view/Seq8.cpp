#include "../controller/Seq8.hpp"
#include "../../deps/rack-components/jacks.hpp"
#include "../../deps/rack-components/screws.hpp"
#include "knobs.hpp"

namespace rack_plugin_SynthKit {

struct Seq8Widget : ModuleWidget {
  Seq8Widget(Seq8Module *module);
};

Seq8Widget::Seq8Widget(Seq8Module *module) : ModuleWidget(module) {
  box.size = Vec(5 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

  {
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/Seq8.svg")));
    addChild(panel);
  }

  addChild(Widget::create<JLHHexScrew>(Vec(1, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(61, 1)));
  addChild(Widget::create<JLHHexScrew>(Vec(1, 366)));
  addChild(Widget::create<JLHHexScrew>(Vec(61, 366)));

  addInput(Port::create<RCJackSmallLight>(Vec(25.23, 73), Port::INPUT, module,
                                         Seq8Module::CLOCK_INPUT));
  addOutput(Port::create<RCJackSmallDark>(Vec(25.23, 305), Port::OUTPUT, module,
                                          Seq8Module::GATE_OUTPUT));

  addParam(ParamWidget::create<Knob19Snap>(
      Vec(15, 118), module, Seq8Module::OCTAVE1_PARAM, 0.0, 8.0, 4.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(15, 139), module, Seq8Module::OCTAVE2_PARAM, 0.0, 8.0, 4.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(15, 160), module, Seq8Module::OCTAVE3_PARAM, 0.0, 8.0, 4.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(15, 181), module, Seq8Module::OCTAVE4_PARAM, 0.0, 8.0, 4.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(15, 202), module, Seq8Module::OCTAVE5_PARAM, 0.0, 8.0, 4.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(15, 223), module, Seq8Module::OCTAVE6_PARAM, 0.0, 8.0, 4.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(15, 244), module, Seq8Module::OCTAVE7_PARAM, 0.0, 8.0, 4.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(15, 265), module, Seq8Module::OCTAVE8_PARAM, 0.0, 8.0, 4.0));

  addParam(ParamWidget::create<Knob19Snap>(
      Vec(41, 118), module, Seq8Module::SEQ1_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(41, 139), module, Seq8Module::SEQ2_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(41, 160), module, Seq8Module::SEQ3_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(41, 181), module, Seq8Module::SEQ4_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(41, 202), module, Seq8Module::SEQ5_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(41, 223), module, Seq8Module::SEQ6_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(41, 244), module, Seq8Module::SEQ7_PARAM, 0.0, 11.0, 5.0));
  addParam(ParamWidget::create<Knob19Snap>(
      Vec(41, 265), module, Seq8Module::SEQ8_PARAM, 0.0, 11.0, 5.0));

  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(66.5, 124.28), module, Seq8Module::FIRST_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(66.5, 145.28), module, Seq8Module::SECOND_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(66.5, 166.28), module, Seq8Module::THIRD_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(66.5, 187.28), module, Seq8Module::FOURTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(66.5, 208.28), module, Seq8Module::FIFTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(66.5, 229.28), module, Seq8Module::SIXTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(66.5, 250.28), module, Seq8Module::SEVENTH_LED));
  addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(
      Vec(66.5, 271.28), module, Seq8Module::EIGHTH_LED));
}

} // namespace rack_plugin_SynthKit

using namespace rack_plugin_SynthKit;

RACK_PLUGIN_MODEL_INIT(SynthKit, Seq8) {
   Model *modelSeq8 = Model::create<Seq8Module, Seq8Widget>(
      "SynthKit", "8-Step Sequencer", "8-Step Sequencer", SEQUENCER_TAG);
   return modelSeq8;
}
