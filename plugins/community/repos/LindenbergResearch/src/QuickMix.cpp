#include <dsp/functions.hpp>
#include "LindenbergResearch.hpp"

using namespace rack;
using namespace lrt;

namespace rack_plugin_LindenbergResearch {

struct QuickMix : Module {
   enum ParamIds {
      LEVEL1_PARAM,
      LEVEL2_PARAM,
      LEVEL3_PARAM,
      LEVEL4_PARAM,
      LEVEL5_PARAM,
      LEVEL6_PARAM,
      LEVELM_PARAM,
      NUM_PARAMS
   };
   enum InputIds {
      M1_INPUT,
      M2_INPUT,
      M3_INPUT,
      M4_INPUT,
      M5_INPUT,
      M6_INPUT,
      NUM_INPUTS
   };
   enum OutputIds {
      MASTER_OUTPUT,
      NUM_OUTPUTS
   };
   enum LightIds {
      LEVEL1_LIGHT,
      LEVEL2_LIGHT,
      LEVEL3_LIGHT,
      LEVEL4_LIGHT,
      LEVEL5_LIGHT,
      LEVEL6_LIGHT,
      LEVELM_LIGHT,
      NUM_LIGHTS
   };


   QuickMix() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}


   void step() override;
};


void QuickMix::step() {
   float out = 0;
   /* lights */
   for (int i = 0; i < NUM_LIGHTS - 1; i++) {
      lights[i].value = quadraticBipolar(params[i].value) * inputs[i].value / 10;
   }

   /* mixup all signals */
   for (int i = 0; i < NUM_INPUTS; i++) {
      out += inputs[i].value * quadraticBipolar(params[i].value);
   }

   /* master out light */
   lights[LEVELM_LIGHT].value = quadraticBipolar(params[LEVELM_PARAM].value) * out / 10;

   out *= quadraticBipolar(params[LEVELM_PARAM].value);

   outputs[MASTER_OUTPUT].value = out;
}


/**
 * @brief Blank Panel Small
 */
struct QuickMixWidget : LRModuleWidget {
   QuickMixWidget(QuickMix *module);
};


QuickMixWidget::QuickMixWidget(QuickMix *module) : LRModuleWidget(module) {
   panel = new LRPanel();
   panel->setBackground(SVG::load(assetPlugin(plugin, "res/QuickMix.svg")));
   addChild(panel);

   box.size = panel->box.size;

   // ***** SCREWS **********
   addChild(Widget::create<ScrewDarkA>(Vec(4.7f, 1)));
   addChild(Widget::create<ScrewDarkA>(Vec(4.7f, 366)));

   addChild(Widget::create<ScrewDarkA>(Vec(75.f - 19.3f, 1)));
   addChild(Widget::create<ScrewDarkA>(Vec(75.f - 19.3f, 366)));    // ***** SCREWS **********

   // ***** MAIN KNOBS ******
   addParam(ParamWidget::create<LRSmallKnob>(Vec(43, 55.8), module, QuickMix::LEVEL1_PARAM, -1.f, 1.f, 0.f));
   addParam(ParamWidget::create<LRSmallKnob>(Vec(43, 100.8), module, QuickMix::LEVEL2_PARAM, -1.f, 1.f, 0.f));
   addParam(ParamWidget::create<LRSmallKnob>(Vec(43, 145.8), module, QuickMix::LEVEL3_PARAM, -1.f, 1.f, 0.f));
   addParam(ParamWidget::create<LRSmallKnob>(Vec(43, 190.8), module, QuickMix::LEVEL4_PARAM, -1.f, 1.f, 0.f));
   addParam(ParamWidget::create<LRSmallKnob>(Vec(43, 235.8), module, QuickMix::LEVEL5_PARAM, -1.f, 1.f, 0.f));
   addParam(ParamWidget::create<LRSmallKnob>(Vec(43, 280.8), module, QuickMix::LEVEL6_PARAM, -1.f, 1.f, 0.f));

   addParam(ParamWidget::create<LRSmallKnob>(Vec(8, 326.5), module, QuickMix::LEVELM_PARAM, -3.f, 3.f, 1.f));
   // ***** MAIN KNOBS ******

   // ***** INPUTS **********
   addInput(Port::create<LRIOPort>(Vec(7, 55.f), Port::INPUT, module, QuickMix::M1_INPUT));
   addInput(Port::create<LRIOPort>(Vec(7, 100.f), Port::INPUT, module, QuickMix::M2_INPUT));
   addInput(Port::create<LRIOPort>(Vec(7, 145.f), Port::INPUT, module, QuickMix::M3_INPUT));
   addInput(Port::create<LRIOPort>(Vec(7, 190.f), Port::INPUT, module, QuickMix::M4_INPUT));
   addInput(Port::create<LRIOPort>(Vec(7, 235.f), Port::INPUT, module, QuickMix::M5_INPUT));
   addInput(Port::create<LRIOPort>(Vec(7, 280.f), Port::INPUT, module, QuickMix::M6_INPUT));
   // ***** INPUTS **********

   // ***** OUTPUTS *********
   addOutput(Port::create<LRIOPort>(Vec(40, 326.05), Port::OUTPUT, module, QuickMix::MASTER_OUTPUT));
   // ***** OUTPUTS *********


   // ***** LIGHTS **********
   addChild(ModuleLightWidget::create<LRLight>(Vec(33, 81.6), module, QuickMix::LEVEL1_LIGHT));
   addChild(ModuleLightWidget::create<LRLight>(Vec(33, 125.6), module, QuickMix::LEVEL2_LIGHT));
   addChild(ModuleLightWidget::create<LRLight>(Vec(33, 169.6), module, QuickMix::LEVEL3_LIGHT));
   addChild(ModuleLightWidget::create<LRLight>(Vec(33, 214.6), module, QuickMix::LEVEL4_LIGHT));
   addChild(ModuleLightWidget::create<LRLight>(Vec(33, 259.6), module, QuickMix::LEVEL5_LIGHT));
   addChild(ModuleLightWidget::create<LRLight>(Vec(33, 304.6), module, QuickMix::LEVEL6_LIGHT));

   addChild(ModuleLightWidget::create<LRLight>(Vec(33, 344.6), module, QuickMix::LEVELM_LIGHT));
   // ***** LIGHTS **********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, QuickMix) {
   Model *modelQuickMix = Model::create<QuickMix, QuickMixWidget>("Lindenberg Research", "QuickMixer", "Q: Quick Mixer", MIXER_TAG);
   return modelQuickMix;
}
