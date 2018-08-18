//**************************************************************************************
//FoldBack Distortion Module for VCV Rack by Autodafe http://www.autodafe.net
//
//Based on code taken from the Fundamentals plugins by Andrew Belt http://www.vcvrack.com
//And part of code on musicdsp.org: http://musicdsp.org/showArchiveComment.php?ArchiveID=203
//**************************************************************************************

#include "Autodafe.hpp"

namespace rack_plugin_Autodafe {

struct FoldBack : Module {
   enum ParamIds {
      THRESHOLD_PARAM,
      ATTEN_PARAM,
      NUM_PARAMS
   };
   enum InputIds {
      
      INPUT,
      CV_THRESHOLD,
      NUM_INPUTS
   };
   enum OutputIds {
      OUTPUT,
      NUM_OUTPUTS
   };

   FoldBack();
   void step();
};


FoldBack::FoldBack() {
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
}

float foldback(float in, float threshold)
{
   if (in>threshold || in<-threshold)
   {
      in = fabs(fabs(fmod(in - threshold, threshold * 4)) - threshold * 2) - threshold;
   }
   return in;
}

void FoldBack::step() {
   
   float in = inputs[INPUT].value / 5.0f;
   float threshold = params[THRESHOLD_PARAM].value;
   float coeff = inputs[CV_THRESHOLD].value * params[ATTEN_PARAM].value / 5.0f;

   outputs[OUTPUT].value= 5.0f* foldback(in, threshold+coeff);
}

struct FoldBackWidget : ModuleWidget{
   FoldBackWidget(FoldBack *module);
};

FoldBackWidget::FoldBackWidget(FoldBack *module) : ModuleWidget(module) {
   box.size = Vec(15 * 6, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/FoldBack.svg")));
      addChild(panel);
   }

   addChild(createScrew<ScrewSilver>(Vec(5, 0)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 0)));
   addChild(createScrew<ScrewSilver>(Vec(5, 365)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 365)));

   addParam(createParam<AutodafeKnobGreenBig>(Vec(18, 61), module, FoldBack::THRESHOLD_PARAM, 0.1, 1.0, 1.0));

   addInput(createInput<PJ301MPort>(Vec(32, 150), module, FoldBack::CV_THRESHOLD));

   addParam(createParam<AutodafeKnobGreen>(Vec(27, 190), module, FoldBack::ATTEN_PARAM, -1.0, 1.0, 0.0));
   addInput(createInput<PJ301MPort>(Vec(10, 320), module, FoldBack::INPUT));

   addOutput(createOutput<PJ301MPort>(Vec(48, 320), module, FoldBack::OUTPUT));
}

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;

RACK_PLUGIN_MODEL_INIT(Autodafe, FoldBack) {
   return Model::create<FoldBack, FoldBackWidget>("Autodafe",  "Foldback Distortion", "Foldback Distortion", EFFECT_TAG, WAVESHAPER_TAG, DISTORTION_TAG);
}
