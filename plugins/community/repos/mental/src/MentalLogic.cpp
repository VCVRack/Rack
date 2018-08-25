///////////////////////////////////////////////////
//
//   Logic Gates VCV Module
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "mental.hpp"

namespace rack_plugin_mental {

struct MentalLogic : Module {
	enum ParamIds {
		NUM_PARAMS
	};  
	enum InputIds {		
		INPUT_A_1,
    INPUT_B_1,
    INPUT_A_2,
    INPUT_B_2,
    INPUT_INV_1,
    INPUT_INV_2,
    INPUT_A_3,
    INPUT_B_3,
    INPUT_C_3,
    INPUT_D_3,
    INPUT_E_3,    
    NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_AND_1,
    OUTPUT_OR_1,
    OUTPUT_AND_2,
    OUTPUT_OR_2,
    OUTPUT_INV_1,
    OUTPUT_INV_2,
    OUTPUT_OR_3,        
		NUM_OUTPUTS
	};
  enum LightIds {
		AND_LED_1,
    OR_LED_1,
    AND_LED_2,
    OR_LED_2,
    INV_LED_1,
    INV_LED_2,
    OR_LED_3,
		NUM_LIGHTS
	};
  
	MentalLogic() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void MentalLogic::step()
{
  
  float signal_in_A1 = inputs[INPUT_A_1].value;
  float signal_in_B1 = inputs[INPUT_B_1].value;
  float signal_in_A2 = inputs[INPUT_A_2].value;
  float signal_in_B2 = inputs[INPUT_B_2].value;
  float inv_1_input = inputs[INPUT_INV_1].value;
  float inv_2_input = inputs[INPUT_INV_2].value;
  float or_3_A_input = inputs[INPUT_A_3].value;
  float or_3_B_input = inputs[INPUT_B_3].value;
  float or_3_C_input = inputs[INPUT_C_3].value;
  float or_3_D_input = inputs[INPUT_D_3].value;
  float or_3_E_input = inputs[INPUT_E_3].value;
  
  if (inv_1_input > 0.0)
  { 
    outputs[OUTPUT_INV_1].value = 0.0;
    lights[INV_LED_1].value = 0.0;
  }
  else
  {
    outputs[OUTPUT_INV_1].value = 10.0;
    lights[INV_LED_1].value = 1.0;
  }
  if (inv_2_input > 0.0)
  { 
    outputs[OUTPUT_INV_2].value = 0.0;
    lights[INV_LED_2].value = 0.0;
  }
  else
  {
    outputs[OUTPUT_INV_2].value = 10.0;
    lights[INV_LED_2].value = 1.0;
  }
    
  //////////////////////////
    
  if (signal_in_A1 > 0.0 && signal_in_B1 > 0.0 )
  {
    outputs[OUTPUT_AND_1].value = 10.0;    
    lights[AND_LED_1].value = 1.0;
  }
  else 
  {
    outputs[OUTPUT_AND_1].value = 0.0;    
    lights[AND_LED_1].value = 0.0;
  }
  if (signal_in_A1 > 0.0 || signal_in_B1 > 0.0 )
  {
    outputs[OUTPUT_OR_1].value = 10.0;
    lights[OR_LED_1].value = 1.0;
  }
  else 
  {
    outputs[OUTPUT_OR_1].value = 0.0;    
    lights[OR_LED_1].value = 0.0;
  }
  //////////////////////////////////////
  if (signal_in_A2 > 0.0 && signal_in_B2 > 0.0 )
  {
    outputs[OUTPUT_AND_2].value = 10.0;    
    lights[AND_LED_2].value = 1.0;
  }
  else 
  {
    outputs[OUTPUT_AND_2].value = 0.0;    
    lights[AND_LED_2].value = 0.0;
  }
  if (signal_in_A2 > 0.0 || signal_in_B2 > 0.0 )
  {
    outputs[OUTPUT_OR_2].value = 10.0;
    lights[OR_LED_2].value = 1.0;
  }
  else 
  {
    outputs[OUTPUT_OR_2].value = 0.0;    
    lights[OR_LED_2].value = 0.0;
  } 
  //////////////// Big or
  if ( or_3_A_input > 0.0 || or_3_B_input > 0.0 || or_3_C_input > 0.0 || or_3_D_input > 0.0 || or_3_E_input > 0.0 )
  {
    outputs[OUTPUT_OR_3].value = 10.0;
    lights[OR_LED_3].value = 1.0;
  }
  else 
  {
    outputs[OUTPUT_OR_3].value = 0.0;    
    lights[OR_LED_3].value = 0.0;
  } 
}

/////////////////////////
struct MentalLogicWidget : ModuleWidget {
  MentalLogicWidget(MentalLogic *module);
};

MentalLogicWidget::MentalLogicWidget(MentalLogic *module) : ModuleWidget(module)
{
	
  setPanel(SVG::load(assetPlugin(plugin, "res/MentalLogic.svg")));

  int input_column = 3;
  int output_column = 28;
  int led_column = 58;
  int first_row = 25;
  int row_spacing = 25;
  int vert_offset = 60;
  
  addInput(Port::create<GateInPort>(Vec(input_column, first_row), Port::INPUT, module, MentalLogic::INPUT_A_1));
  addInput(Port::create<GateInPort>(Vec(input_column, first_row+row_spacing), Port::INPUT, module, MentalLogic::INPUT_B_1));  
  
  addOutput(Port::create<GateOutPort>(Vec(output_column, first_row), Port::OUTPUT, module, MentalLogic::OUTPUT_AND_1));
  addOutput(Port::create<GateOutPort>(Vec(output_column, first_row+row_spacing), Port::OUTPUT, module, MentalLogic::OUTPUT_OR_1));  
  
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(led_column, first_row + 8), module, MentalLogic::AND_LED_1));
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(led_column, first_row+row_spacing + 8), module, MentalLogic::OR_LED_1));
  
  ////////////////////////////
  
  addInput(Port::create<GateInPort>(Vec(input_column, vert_offset + first_row), Port::INPUT, module, MentalLogic::INPUT_A_2));
  addInput(Port::create<GateInPort>(Vec(input_column, vert_offset + first_row + row_spacing), Port::INPUT, module, MentalLogic::INPUT_B_2));  
  
  addOutput(Port::create<GateOutPort>(Vec(output_column, vert_offset + first_row), Port::OUTPUT, module, MentalLogic::OUTPUT_AND_2));
  addOutput(Port::create<GateOutPort>(Vec(output_column, vert_offset + first_row + row_spacing), Port::OUTPUT, module, MentalLogic::OUTPUT_OR_2));  
  
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(led_column, vert_offset +  first_row + 8), module, MentalLogic::AND_LED_2));
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(led_column, vert_offset +first_row + row_spacing + 8), module, MentalLogic::OR_LED_2));
  
  ///// Inverters
  
  addInput(Port::create<GateInPort>(Vec(input_column, vert_offset * 2 + first_row), Port::INPUT, module, MentalLogic::INPUT_INV_1));
  addInput(Port::create<GateInPort>(Vec(input_column, vert_offset * 2 + first_row + row_spacing), Port::INPUT, module, MentalLogic::INPUT_INV_2));  
  
  addOutput(Port::create<GateOutPort>(Vec(output_column, vert_offset * 2 + first_row), Port::OUTPUT, module, MentalLogic::OUTPUT_INV_1));
  addOutput(Port::create<GateOutPort>(Vec(output_column, vert_offset * 2 + first_row + row_spacing), Port::OUTPUT, module, MentalLogic::OUTPUT_INV_2));
  
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(led_column, vert_offset * 2 + first_row + 8), module, MentalLogic::INV_LED_1));
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(led_column, vert_offset * 2 + first_row + row_spacing + 8), module, MentalLogic::INV_LED_2));
  
  ////// Big or
  addInput(Port::create<GateInPort>(Vec(input_column, vert_offset + 150), Port::INPUT, module, MentalLogic::INPUT_A_3));
  addInput(Port::create<GateInPort>(Vec(input_column, vert_offset + 175), Port::INPUT, module, MentalLogic::INPUT_B_3));
  addInput(Port::create<GateInPort>(Vec(input_column, vert_offset + 200), Port::INPUT, module, MentalLogic::INPUT_C_3));
  addInput(Port::create<GateInPort>(Vec(input_column, vert_offset + 225), Port::INPUT, module, MentalLogic::INPUT_D_3));
  addInput(Port::create<GateInPort>(Vec(input_column, vert_offset + 250), Port::INPUT, module, MentalLogic::INPUT_E_3));  
  
	addOutput(Port::create<GateOutPort>(Vec(output_column, vert_offset + 150), Port::OUTPUT, module, MentalLogic::OUTPUT_OR_3));
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(led_column, vert_offset + 158), module, MentalLogic::OR_LED_3));   
}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalLogic) {
   Model *modelMentalLogic = Model::create<MentalLogic, MentalLogicWidget>("mental", "MentalLogic", "Logic Gates", UTILITY_TAG);
   return modelMentalLogic;
}
