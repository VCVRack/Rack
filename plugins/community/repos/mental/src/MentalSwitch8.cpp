///////////////////////////////////////////////////
//
//   8 way switch with Binary Decoder selector VCV Module
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "mental.hpp"

namespace rack_plugin_mental {

struct MentalSwitch8 : Module {
	enum ParamIds {
    NUM_PARAMS
	};  
	enum InputIds {
    SIG_INPUT,
    INPUT_1,
    INPUT_2,
    INPUT_4,		  
		NUM_INPUTS
	};
	enum OutputIds {
    OUTPUT,    
		NUM_OUTPUTS = OUTPUT + 8
	};
  enum LightIds {
    OUTPUT_LEDS,
    NUM_LIGHTS = OUTPUT_LEDS + 8
  };
    
  float in_1 = 0.0;
  float in_2 = 0.0;
  float in_4 = 0.0;
  
  int one, two, four, decoded;
  
	MentalSwitch8() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;  
};

void MentalSwitch8::step()
{
  for ( int i = 0 ; i < 8 ; i ++)
  {
    lights[OUTPUT_LEDS + i].value = 0.0;
    outputs[OUTPUT + i].value = 0.0;
  }
  
  in_1 = inputs[INPUT_1].value;
  in_2 = inputs[INPUT_2].value;
  in_4 = inputs[INPUT_4].value;
  
  if (in_1 > 0.0 ) 
  {
    one = 1;
  } else
  {
    one = 0;
  }
  if (in_2 > 0.0) 
  {
    two = 2;
  } else
  {
    two = 0;
  }
  if (in_4 > 0.0) 
  {
    four = 4;
  } else
  {
    four = 0;
  }
  
  decoded = one + two + four;  
  outputs[OUTPUT + decoded].value = inputs[SIG_INPUT].value;
  lights[OUTPUT_LEDS + decoded].value = 1.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
struct MentalSwitch8Widget : ModuleWidget {
  MentalSwitch8Widget(MentalSwitch8 *module);
};

MentalSwitch8Widget::MentalSwitch8Widget(MentalSwitch8 *module) : ModuleWidget(module)
{

  setPanel(SVG::load(assetPlugin(plugin, "res/MentalSwitch8.svg")));
	
  int spacing = 25; 
  int top_space = 15;
  
  addInput(Port::create<GateInPort>(Vec(3, top_space), Port::INPUT, module, MentalSwitch8::INPUT_1));
  addInput(Port::create<GateInPort>(Vec(3, top_space + spacing), Port::INPUT, module, MentalSwitch8::INPUT_2));
  addInput(Port::create<GateInPort>(Vec(3, top_space + spacing * 2), Port::INPUT, module, MentalSwitch8::INPUT_4));
  
  addInput(Port::create<InPort>(Vec(3, top_space + spacing * 3 + 15), Port::INPUT, module, MentalSwitch8::SIG_INPUT));
  
  for (int i = 0; i < 8 ; i++)
  {  
   addOutput(Port::create<OutPort>(Vec(30, top_space + spacing * i), Port::OUTPUT, module, MentalSwitch8::OUTPUT + i));   	 
   addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(60, top_space +  spacing * i + 8), module,MentalSwitch8::OUTPUT_LEDS + i));
  }
  
}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalSwitch8) {
   Model *modelMentalSwitch8 = Model::create<MentalSwitch8, MentalSwitch8Widget>("mental", "MentalSwitch8", "8 Way Switch", SWITCH_TAG, UTILITY_TAG);
   return modelMentalSwitch8;
}
