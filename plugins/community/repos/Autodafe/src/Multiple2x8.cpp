//**************************************************************************************
//2x8 Multimple Module for VCV Rack by Autodafe http://www.autodafe.net
//
//**************************************************************************************

#include "Autodafe.hpp"

namespace rack_plugin_Autodafe {

struct Multiple28 : Module {
   enum ParamIds {
      
      NUM_PARAMS
   };

   enum InputIds {
      INPUT1,
      INPUT2,
      NUM_INPUTS
   };

   enum OutputIds {
      OUT11,
      OUT12,
      OUT13,
      OUT14,
      OUT15,
      OUT16,
      OUT17,
      OUT18,
      OUT21,
      OUT22,
      OUT23,
      OUT24,
      OUT25,
      OUT26,
      OUT27,
      OUT28,
      NUM_OUTPUTS
   };

   Multiple28();
   void step();
};


Multiple28::Multiple28() {
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
}


void Multiple28::step() {
   
   float IN1 = inputs[INPUT1].value;
   float IN2 = inputs[INPUT2].value;

   // Set outputs
   //first column
   if (outputs[OUT11].active) {
      outputs[OUT11].value = IN1;
   }
   
   if (outputs[OUT12].active) {
      outputs[OUT12].value = IN1;
   }

   if (outputs[OUT13].active) {
      outputs[OUT13].value = IN1;
   }

   if (outputs[OUT14].active) {
      outputs[OUT14].value = IN1;
   }

   if (outputs[OUT15].active) {
      outputs[OUT15].value = IN1;
   }

   if (outputs[OUT16].active) {
      outputs[OUT16].value = IN1;
   }

   if (outputs[OUT17].active) {
      outputs[OUT17].value = IN1;
   }

   if (outputs[OUT18].active) {
      outputs[OUT18].value = IN1;
   }


   //SECOND COLUMN
   if (outputs[OUT21].active) {
      outputs[OUT21].value = IN2;
   }

   if (outputs[OUT22].active) {
      outputs[OUT22].value = IN2;
   }

   if (outputs[OUT23].active) {
      outputs[OUT23].value = IN2;
   }

   if (outputs[OUT24].active) {
      outputs[OUT24].value = IN2;
   }

   if (outputs[OUT25].active) {
      outputs[OUT25].value = IN2;
   }

   if (outputs[OUT26].active) {
      outputs[OUT26].value = IN2;
   }

   if (outputs[OUT27].active) {
      outputs[OUT27].value = IN2;
   }

   if (outputs[OUT28].active) {
      outputs[OUT28].value = IN2;
   }
}

struct Multiple28Widget : ModuleWidget  {
   Multiple28Widget(Multiple28 *module);
};

Multiple28Widget::Multiple28Widget(Multiple28 *module) : ModuleWidget(module) {
   box.size = Vec(15*6, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;
      panel->setBackground(SVG::load(assetPlugin(plugin, "res/Multiple28.svg")));
      addChild(panel);
   }

   addChild(createScrew<ScrewSilver>(Vec(5, 0)));
   addChild(createScrew<ScrewSilver>(Vec(5, 365)));
   addChild(createScrew<ScrewSilver>(Vec(70, 0)));
   addChild(createScrew<ScrewSilver>(Vec(70, 365)));

   addInput(createInput<PJ3410Port>(Vec(10, 20), module, Multiple28::INPUT1));
   addInput(createInput<PJ3410Port>(Vec(50, 20), module, Multiple28::INPUT2));
   
   addOutput(createOutput<PJ3410Port>(Vec(10, 60), module, Multiple28::OUT11));
   addOutput(createOutput<PJ3410Port>(Vec(10, 95), module, Multiple28::OUT12));
   addOutput(createOutput<PJ3410Port>(Vec(10, 130), module, Multiple28::OUT13));
   addOutput(createOutput<PJ3410Port>(Vec(10, 165), module, Multiple28::OUT14));
   addOutput(createOutput<PJ3410Port>(Vec(10, 200), module, Multiple28::OUT15));
   addOutput(createOutput<PJ3410Port>(Vec(10, 235), module, Multiple28::OUT16));
   addOutput(createOutput<PJ3410Port>(Vec(10, 270), module, Multiple28::OUT17));
   addOutput(createOutput<PJ3410Port>(Vec(10, 305), module, Multiple28::OUT18));

   addOutput(createOutput<PJ3410Port>(Vec(50, 60), module, Multiple28::OUT21));
   addOutput(createOutput<PJ3410Port>(Vec(50, 95), module, Multiple28::OUT22));
   addOutput(createOutput<PJ3410Port>(Vec(50, 130), module, Multiple28::OUT23));
   addOutput(createOutput<PJ3410Port>(Vec(50, 165), module, Multiple28::OUT24));
   addOutput(createOutput<PJ3410Port>(Vec(50, 200), module, Multiple28::OUT25));
   addOutput(createOutput<PJ3410Port>(Vec(50, 235), module, Multiple28::OUT26));
   addOutput(createOutput<PJ3410Port>(Vec(50, 270), module, Multiple28::OUT27));
   addOutput(createOutput<PJ3410Port>(Vec(50, 305), module, Multiple28::OUT28));
}

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;

RACK_PLUGIN_MODEL_INIT(Autodafe, Multiple28) {
   return Model::create<Multiple28, Multiple28Widget>("Autodafe",  "Multiple 2x8", "Multiple 2x8", UTILITY_TAG);
}
