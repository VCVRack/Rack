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
        SHAPE_PARAM,
      LEVELM_PARAM,
      NUM_PARAMS
   };
   enum InputIds {
      M1_INPUT,
      M2_INPUT,
      M3_INPUT,
      M4_INPUT,
      M5_INPUT,
        CV_INPUT,
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
    addChild(Widget::create<ScrewDarkA>(Vec(15, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 1)));
    addChild(Widget::create<ScrewDarkA>(Vec(15, 366)));
    addChild(Widget::create<ScrewDarkA>(Vec(box.size.x - 30, 366)));
    // ***** SCREWS **********

    // ***** MAIN KNOBS ******
    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 53.8), module, QuickMix::LEVEL1_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 88.8), module, QuickMix::LEVEL2_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 123.8), module, QuickMix::LEVEL3_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 158.8), module, QuickMix::LEVEL4_PARAM, -1.f, 1.f, 0.f));
    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 193.8), module, QuickMix::LEVEL5_PARAM, -1.f, 1.f, 0.f));

    addParam(ParamWidget::create<LRSmallKnob>(Vec(62.3, 242.0), module, QuickMix::SHAPE_PARAM, -1.f, 1.f, 0.f));

    addParam(ParamWidget::create<LRSmallKnob>(Vec(18.8, 305.8), module, QuickMix::LEVELM_PARAM, -3.f, 3.f, 1.f));
    // ***** MAIN KNOBS ******

    // ***** INPUTS **********
    addInput(Port::create<LRIOPort>(Vec(16.5, 52.6), Port::INPUT, module, QuickMix::M1_INPUT));
    addInput(Port::create<LRIOPort>(Vec(16.5, 87.6), Port::INPUT, module, QuickMix::M2_INPUT));
    addInput(Port::create<LRIOPort>(Vec(16.5, 122.6), Port::INPUT, module, QuickMix::M3_INPUT));
    addInput(Port::create<LRIOPort>(Vec(16.5, 157.6), Port::INPUT, module, QuickMix::M4_INPUT));
    addInput(Port::create<LRIOPort>(Vec(16.5, 192.6), Port::INPUT, module, QuickMix::M5_INPUT));

    addInput(Port::create<LRIOPort>(Vec(16.5, 240.8f), Port::INPUT, module, QuickMix::CV_INPUT));
    // ***** INPUTS **********

    // ***** OUTPUTS *********
    addOutput(Port::create<LRIOPort>(Vec(60.9, 304.8), Port::OUTPUT, module, QuickMix::MASTER_OUTPUT));
   // ***** OUTPUTS *********


    // ***** LIGHTS **********
    addChild(ModuleLightWidget::create<LRLight>(Vec(47.5, 61.3), module, QuickMix::LEVEL1_LIGHT));
    addChild(ModuleLightWidget::create<LRLight>(Vec(47.5, 96.3), module, QuickMix::LEVEL2_LIGHT));
    addChild(ModuleLightWidget::create<LRLight>(Vec(47.5, 131.3), module, QuickMix::LEVEL3_LIGHT));
    addChild(ModuleLightWidget::create<LRLight>(Vec(47.5, 166.3), module, QuickMix::LEVEL4_LIGHT));
    addChild(ModuleLightWidget::create<LRLight>(Vec(47.5, 201.3), module, QuickMix::LEVEL5_LIGHT));


    // addChild(ModuleLightWidget::create<LRLight>(Vec(47.5, 304.6), module, QuickMix::LEVEL6_LIGHT));

    // ***** LIGHTS **********
}

} // namespace rack_plugin_LindenbergResearch

using namespace rack_plugin_LindenbergResearch;

RACK_PLUGIN_MODEL_INIT(LindenbergResearch, QuickMix) {
   Model *modelQuickMix = Model::create<QuickMix, QuickMixWidget>("Lindenberg Research", "QuickMixer", "Q: Quick Mixer", MIXER_TAG);
   return modelQuickMix;
}
