//**************************************************************************************
//BitCrusher Module for VCV Rack by Autodafe http://www.autodafe.net
//
//Based on code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//And part of code on musicdsp.org: http://musicdsp.org/showArchiveComment.php?ArchiveID=78
//**************************************************************************************


#include "Autodafe.hpp"
#include <stdlib.h>

#include "stk/include/Chorus.h"

using namespace stk;

namespace rack_plugin_Autodafe {

struct ChorusFx : Module{
   enum ParamIds {
      PARAM_RATE,
      PARAM_FEEDBACK,
      PARAM_DEPTH,
      NUM_PARAMS
   };
   enum InputIds {
      RATE_CV_IN,
      DEPTH_CV_IN,
      INPUT,
      NUM_INPUTS
   };
   enum OutputIds {
      OUT,
      NUM_OUTPUTS
   };


   ChorusFx();

     
   Chorus *cho = new Chorus(1000); // OK

   void step();
};


ChorusFx::ChorusFx() {
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
}

void ChorusFx::step() {

   StkFloat  rate = params[PARAM_RATE].value;
   
   StkFloat depth = params[PARAM_DEPTH].value;

   StkFloat  input = inputs[INPUT].value / 5.0;


   cho->setModFrequency(rate);
   cho->setModDepth (depth);
   
   cho->tick(input,0);

   outputs[OUT].value= cho->lastOut(0) * 5;
}

struct ChorusFxWidget : ModuleWidget{
   ChorusFxWidget(ChorusFx *module);
};

ChorusFxWidget::ChorusFxWidget(ChorusFx *module) : ModuleWidget(module) {
   box.size = Vec(15 * 6, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Chorus.svg")));
      addChild(panel);
   }

   addChild(createScrew<ScrewSilver>(Vec(1, 0)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 0)));
   addChild(createScrew<ScrewSilver>(Vec(1, 365)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 365)));

   addParam(createParam<AutodafeKnobGreenBig>(Vec(20, 60), module, ChorusFx::PARAM_RATE, 0, 1, 0));
   addParam(createParam<AutodafeKnobGreen>(Vec(27, 140), module, ChorusFx::PARAM_DEPTH, 0, 1, 0));

   addInput(createInput<PJ301MPort>(Vec(10, 320), module, ChorusFx::INPUT));
   addOutput(createOutput<PJ301MPort>(Vec(48, 320), module, ChorusFx::OUT));
   
}

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;

RACK_PLUGIN_MODEL_INIT(Autodafe, ChorusFx) {
   return Model::create<ChorusFx, ChorusFxWidget>("Autodafe",  "Chorus", "Chorus", EFFECT_TAG);
}
