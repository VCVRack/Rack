///////////////////////////////////////////////////
//
//   Multiplexers VCV Module
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "mental.hpp"

namespace rack_plugin_mental {

struct MentalMuxes : Module {
	enum ParamIds {
		NUM_PARAMS
	};  
	enum InputIds {		
		INPUT_1A,
    INPUT_2A,
    SELECT_A,
    INPUT_1B,
    INPUT_2B,
    SELECT_B, 
    INPUT_1C,
    INPUT_2C,
    INPUT_3C,
    INPUT_4C,
    SELECT_C,     
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_A,
    OUTPUT_B,
    OUTPUT_C,        
		NUM_OUTPUTS
	};
  enum LightIds {
    LEVEL_LED_A1,
    LEVEL_LED_A2,
    LEVEL_LED_B1,
    LEVEL_LED_B2,
    LEVEL_LED_C1,
    LEVEL_LED_C2,
    LEVEL_LED_C3,
    LEVEL_LED_C4,    
    NUM_LIGHTS
  };
  
	MentalMuxes() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void MentalMuxes::step()
{
  
  float signal_in_a1 = inputs[INPUT_1A].value;
  float signal_in_a2 = inputs[INPUT_2A].value;
  float select_a = inputs[SELECT_A].value;
  
  if (select_a > 0.0 )
  {
    outputs[OUTPUT_A].value = signal_in_a2;    
    lights[LEVEL_LED_A2].value = std::abs((signal_in_a2 / 3));
    lights[LEVEL_LED_A1].value = 0.0;
  }
  else
  {
    outputs[OUTPUT_A].value = signal_in_a1;    
    lights[LEVEL_LED_A1].value = std::abs((signal_in_a1 / 3));
    lights[LEVEL_LED_A2].value = 0.0;
  }
  float signal_in_b1 = inputs[INPUT_1B].value;
  float signal_in_b2 = inputs[INPUT_2B].value;
  float select_b = inputs[SELECT_B].value;
  
  if (select_b > 0.0 )
  {
    outputs[OUTPUT_B].value = signal_in_b2;    
    lights[LEVEL_LED_B2].value = std::abs((signal_in_b2 / 3));
    lights[LEVEL_LED_B1].value = 0.0;
  }
  else
  {
    outputs[OUTPUT_B].value = signal_in_b1;    
    lights[LEVEL_LED_B1].value = std::abs((signal_in_b1 / 3));
    lights[LEVEL_LED_B2].value = 0.0;
  }
  
  float signal_in_c1 = inputs[INPUT_1C].value;
  float signal_in_c2 = inputs[INPUT_2C].value;
  float signal_in_c3 = inputs[INPUT_3C].value;
  float signal_in_c4 = inputs[INPUT_4C].value;
  float select_c = inputs[SELECT_C].value;
  int selector = round(std::abs(select_c));
  if (selector > 3) selector = 3;
    
  if (selector == 0 )
  {
    outputs[OUTPUT_C].value = signal_in_c1;    
    lights[LEVEL_LED_C1].value = std::abs((signal_in_c1 / 3));
    lights[LEVEL_LED_C2].value = 0.0;
    lights[LEVEL_LED_C3].value = 0.0;
    lights[LEVEL_LED_C4].value = 0.0;
       
  }
  if (selector == 1 )
  {
    outputs[OUTPUT_C].value = signal_in_c2;    
    lights[LEVEL_LED_C2].value = std::abs((signal_in_c2 / 3));
    lights[LEVEL_LED_C1].value = 0.0;
    lights[LEVEL_LED_C3].value = 0.0;
    lights[LEVEL_LED_C4].value = 0.0;
  }
  if (selector == 2 )
  {
    outputs[OUTPUT_C].value = signal_in_c3;    
    lights[LEVEL_LED_C3].value = std::abs((signal_in_c3 / 3));
    lights[LEVEL_LED_C2].value = 0.0;
    lights[LEVEL_LED_C2].value = 0.0;
    lights[LEVEL_LED_C4].value = 0.0;    
  }
  if (selector == 3 )
  {
    outputs[OUTPUT_C].value = signal_in_c4;    
    lights[LEVEL_LED_C4].value = std::abs((signal_in_c4 / 3));
    lights[LEVEL_LED_C1].value = 0.0;
    lights[LEVEL_LED_C2].value = 0.0;
    lights[LEVEL_LED_C3].value = 0.0;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////
struct MentalMuxesWidget : ModuleWidget {
  MentalMuxesWidget(MentalMuxes *module);
};

MentalMuxesWidget::MentalMuxesWidget(MentalMuxes *module) : ModuleWidget(module)
{

  setPanel(SVG::load(assetPlugin(plugin, "res/MentalMuxes.svg")));

	int group_offset = 90;
  addInput(Port::create<GateInPort>(Vec(3, 75), Port::INPUT, module, MentalMuxes::SELECT_A));  
	addInput(Port::create<InPort>(Vec(3, 25), Port::INPUT, module, MentalMuxes::INPUT_1A));
  addInput(Port::create<InPort>(Vec(3, 50), Port::INPUT, module, MentalMuxes::INPUT_2A));  
  
  addOutput(Port::create<OutPort>(Vec(33, 75), Port::OUTPUT, module, MentalMuxes::OUTPUT_A));
  
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(41, 32), module, MentalMuxes::LEVEL_LED_A1));
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(41, 58), module, MentalMuxes::LEVEL_LED_A2));
  
  addInput(Port::create<GateInPort>(Vec(3, group_offset + 75), Port::INPUT, module, MentalMuxes::SELECT_B));  
	addInput(Port::create<InPort>(Vec(3, group_offset + 25), Port::INPUT, module, MentalMuxes::INPUT_1B));
  addInput(Port::create<InPort>(Vec(3, group_offset + 50), Port::INPUT, module, MentalMuxes::INPUT_2B));  
  
  addOutput(Port::create<OutPort>(Vec(33,group_offset + 75), Port::OUTPUT, module, MentalMuxes::OUTPUT_B));
  
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(41,group_offset + 32), module, MentalMuxes::LEVEL_LED_B1));
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(41,group_offset + 58), module, MentalMuxes::LEVEL_LED_B2));
  
  addInput(Port::create<CVInPort>(Vec(3, group_offset * 2 + 125), Port::INPUT, module, MentalMuxes::SELECT_C));  
	addInput(Port::create<InPort>(Vec(3, group_offset * 2 + 25), Port::INPUT, module, MentalMuxes::INPUT_1C));
  addInput(Port::create<InPort>(Vec(3, group_offset * 2 + 50), Port::INPUT, module, MentalMuxes::INPUT_2C));
  addInput(Port::create<InPort>(Vec(3, group_offset * 2 + 75), Port::INPUT, module, MentalMuxes::INPUT_3C));
  addInput(Port::create<InPort>(Vec(3, group_offset * 2 + 100), Port::INPUT, module, MentalMuxes::INPUT_4C));    
  
  addOutput(Port::create<OutPort>(Vec(33,group_offset * 2 + 125), Port::OUTPUT, module, MentalMuxes::OUTPUT_C));
  
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(41,group_offset * 2 + 32), module, MentalMuxes::LEVEL_LED_C1));
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(41,group_offset * 2 + 58), module, MentalMuxes::LEVEL_LED_C2));
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(41,group_offset * 2 + 82), module, MentalMuxes::LEVEL_LED_C3));
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(41,group_offset * 2 + 108), module, MentalMuxes::LEVEL_LED_C4));
	  
}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalMuxes) {
   Model *modelMentalMuxes = Model::create<MentalMuxes, MentalMuxesWidget>("mental", "MentalMuxes", "Multiplexers", DUAL_TAG, UTILITY_TAG);
   return modelMentalMuxes;
}
