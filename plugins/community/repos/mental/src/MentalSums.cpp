///////////////////////////////////////////////////
//
//   Mental Sums summing - VCV Module
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "mental.hpp"

namespace rack_plugin_mental {

struct MentalSums : Module {
	enum ParamIds {
    
		NUM_PARAMS
	};  
	enum InputIds {
    INPUT_1,
    INPUT_2,
    INPUT_3,
    INPUT_4,
    INPUT_5,
    INPUT_2_1,
    INPUT_2_2,
    INPUT_2_3,
    INPUT_2_4,
    INPUT_2_5,			  
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
    OUTPUT_2,       
		NUM_OUTPUTS
	};
  
  float sum_out_1 = 0.0;
  float sum_out_2 = 0.0;
  
  
	MentalSums() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};

void MentalSums::step()
{
  sum_out_1 = 0.0;
  sum_out_2 = 0.0;
  
  for (int i = 0 ; i < 5 ; i++)
  {
    sum_out_1 += inputs[INPUT_1 + i].value;
    sum_out_2 += inputs[INPUT_2_1 + i].value;
  }
  
  outputs[OUTPUT_1].value = sum_out_1;
	outputs[OUTPUT_2].value = sum_out_2;
  
}

////////////////////////////////////////////////////////////////
struct MentalSumsWidget : ModuleWidget {
	MentalSumsWidget(MentalSums *module);
};

MentalSumsWidget::MentalSumsWidget(MentalSums *module) : ModuleWidget(module)
{

  setPanel(SVG::load(assetPlugin(plugin, "res/MentalSums.svg")));

  addOutput(Port::create<OutPort>(Vec(3, 22), Port::OUTPUT, module, MentalSums::OUTPUT_1));
  addOutput(Port::create<OutPort>(Vec(3, 190), Port::OUTPUT, module, MentalSums::OUTPUT_2));
  
  addInput(Port::create<InPort>(Vec(3, 58), Port::INPUT, module, MentalSums::INPUT_1));
  addInput(Port::create<InPort>(Vec(3, 83), Port::INPUT, module, MentalSums::INPUT_2));
  addInput(Port::create<InPort>(Vec(3, 108), Port::INPUT, module, MentalSums::INPUT_3));
  addInput(Port::create<InPort>(Vec(3, 133), Port::INPUT, module, MentalSums::INPUT_4));
  addInput(Port::create<InPort>(Vec(3, 158), Port::INPUT, module, MentalSums::INPUT_5));
  
  addInput(Port::create<InPort>(Vec(3, 230), Port::INPUT, module, MentalSums::INPUT_2_1));
  addInput(Port::create<InPort>(Vec(3, 255), Port::INPUT, module, MentalSums::INPUT_2_2));
  addInput(Port::create<InPort>(Vec(3, 280), Port::INPUT, module, MentalSums::INPUT_2_3));
  addInput(Port::create<InPort>(Vec(3, 305), Port::INPUT, module, MentalSums::INPUT_2_4));
  addInput(Port::create<InPort>(Vec(3, 330), Port::INPUT, module, MentalSums::INPUT_2_5));	
}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalSums) {
   Model *modelMentalSums = Model::create<MentalSums, MentalSumsWidget>("mental", "MentalSums", "Summing Mixer", MIXER_TAG, UTILITY_TAG);
   return modelMentalSums;
}
