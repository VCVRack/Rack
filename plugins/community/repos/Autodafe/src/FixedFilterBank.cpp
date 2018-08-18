//**************************************************************************************
//Clock Divider Module for VCV Rack by Autodafe http://www.autodafe.net
//
//  Based on code created by Created by Nigel Redmon 
//  EarLevel Engineering: earlevel.com
//  Copyright 2012 Nigel Redmon
//  http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
//**************************************************************************************

#include "Autodafe.hpp"
#include <stdlib.h>

namespace rack_plugin_Autodafe {

struct FixedFilter : Module{
   enum ParamIds {
      EQ1,
      EQ2,
      EQ3,
      EQ4,
      EQ5,
      EQ6,
      EQ7,
      EQ8,

      NUM_PARAMS
   };
   enum InputIds {
      DRIVE_INPUT,
      INPUT,
      NUM_INPUTS
   };
   enum OutputIds {
      OUT,
      NUM_OUTPUTS
   };


   FixedFilter();

   Biquad *bq1 = new Biquad();
   Biquad *bq2 = new Biquad();
   Biquad *bq3 = new Biquad();
   Biquad *bq4 = new Biquad();
   Biquad *bq5 = new Biquad();
   Biquad *bq6 = new Biquad();
   Biquad *bq7 = new Biquad();
   Biquad *bq8 = new Biquad();

   
   void step();
};


FixedFilter::FixedFilter() {
   params.resize(NUM_PARAMS);
   inputs.resize(NUM_INPUTS);
   outputs.resize(NUM_OUTPUTS);
}


void FixedFilter::step() {

   float input = inputs[INPUT].value / 5.0f;
   float out;

   bq1->setBiquad(bq_type_peak, 75.0 / engineGetSampleRate(), 5, params[EQ1].value);
   bq2->setBiquad(bq_type_peak, 125.0 / engineGetSampleRate(), 5, params[EQ2].value);
   bq3->setBiquad(bq_type_peak, 250.0 / engineGetSampleRate(), 5, params[EQ3].value);
   bq4->setBiquad(bq_type_peak, 500.0 / engineGetSampleRate(), 5, params[EQ4].value);
   bq5->setBiquad(bq_type_peak, 1000.0 / engineGetSampleRate(), 5, params[EQ5].value);
   bq6->setBiquad(bq_type_peak, 2000.0 / engineGetSampleRate(), 5, params[EQ6].value);
   bq7->setBiquad(bq_type_peak, 4000.0 / engineGetSampleRate(), 5, params[EQ7].value);
   bq8->setBiquad(bq_type_peak, 8000.0 / engineGetSampleRate(), 5, params[EQ8].value);
   
   out = bq1->process(input);
   out = bq2->process(out);
   out = bq3->process(out);
   out = bq4->process(out);
   out = bq5->process(out);
   out = bq6->process(out);
   out = bq7->process(out);
   out = bq8->process(out);

   outputs[OUT].value= out*5.0f;
}

struct FixedFilterWidget : ModuleWidget{
	FixedFilterWidget(FixedFilter *module);
};

FixedFilterWidget::FixedFilterWidget(FixedFilter *module) : ModuleWidget(module) {
   box.size = Vec(15 * 9, 380);

   {
      SVGPanel *panel = new SVGPanel();
      panel->box.size = box.size;

      panel->setBackground(SVG::load(assetPlugin(plugin, "res/FixedFilterBank.svg")));
      addChild(panel);
   }
   
   addChild(createScrew<ScrewSilver>(Vec(5, 0)));
   addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 0)));
   addChild(createScrew<ScrewSilver>(Vec(5, 365))); 
   addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 365)));


   addParam(createParam<AutodafeKnobBlue>(Vec(20, 50), module, FixedFilter::EQ1, -12, 12.0, 0));
   addParam(createParam<AutodafeKnobBlue>(Vec(20, 110), module, FixedFilter::EQ2, -12, 12.0, 0.0));
   addParam(createParam<AutodafeKnobBlue>(Vec(20, 170), module, FixedFilter::EQ3, -12, 12.0, 0.0));
   addParam(createParam<AutodafeKnobBlue>(Vec(20, 230), module, FixedFilter::EQ4, -12, 12.0, 0.0));
    

   addParam(createParam<AutodafeKnobBlue>(Vec(80, 50), module, FixedFilter::EQ1, -12, 12.0, 0.0));
   addParam(createParam<AutodafeKnobBlue>(Vec(80, 110), module, FixedFilter::EQ2, -12, 12.0, 0.0));
   addParam(createParam<AutodafeKnobBlue>(Vec(80, 170), module, FixedFilter::EQ3, -12, 12.0, 0.0));
   addParam(createParam<AutodafeKnobBlue>(Vec(80, 230), module, FixedFilter::EQ4, -12, 12.0, 0.0));

   addInput(createInput<PJ301MPort>(Vec(25, 320), module, FixedFilter::INPUT));
   
   addOutput(createOutput<PJ301MPort>(Vec(85, 320), module, FixedFilter::OUT));
}

} // namespace rack_plugin_Autodafe

using namespace rack_plugin_Autodafe;

RACK_PLUGIN_MODEL_INIT(Autodafe, FixedFilter) {
   return Model::create<FixedFilter, FixedFilterWidget>("Autodafe", "Fixed Filter Bank", "Fixed Filter Bank", FILTER_TAG);
}
