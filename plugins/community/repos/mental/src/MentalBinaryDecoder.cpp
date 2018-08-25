///////////////////////////////////////////////////
//
//   Binary Decoder VCV Module
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "mental.hpp"

namespace rack_plugin_mental {

struct MentalBinaryDecoder : Module {
	enum ParamIds {
    NUM_PARAMS
	};  
	enum InputIds {
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
  
  int one, two, four, last_led, decoded;
  
	MentalBinaryDecoder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;  
};

void MentalBinaryDecoder::step()
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
  outputs[OUTPUT + decoded].value = 10.0;
  lights[OUTPUT_LEDS + decoded].value = 1.0;  
}

//////////////////////////////////////////////////////////////////////////////
struct MentalBinaryDecoderWidget : ModuleWidget {
  MentalBinaryDecoderWidget(MentalBinaryDecoder *module);
};

MentalBinaryDecoderWidget::MentalBinaryDecoderWidget(MentalBinaryDecoder *module) : ModuleWidget(module)
{
	
  setPanel(SVG::load(assetPlugin(plugin, "res/MentalBinaryDecoder.svg")));

  int spacing = 25; 
  int top_space = 15;
  
  addInput(Port::create<GateInPort>(Vec(3, top_space), Port::INPUT, module, MentalBinaryDecoder::INPUT_1));
  addInput(Port::create<GateInPort>(Vec(3, top_space + spacing), Port::INPUT, module, MentalBinaryDecoder::INPUT_2));
  addInput(Port::create<GateInPort>(Vec(3, top_space + spacing * 2), Port::INPUT, module, MentalBinaryDecoder::INPUT_4));
  
  for (int i = 0; i < 8 ; i++)
  {  
   addOutput(Port::create<GateOutPort>(Vec(30, top_space + spacing * i), Port::OUTPUT, module, MentalBinaryDecoder::OUTPUT + i));   	 
   addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(60, top_space +  spacing * i + 8), module, MentalBinaryDecoder::OUTPUT_LEDS + i));
  }  
}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalBinaryDecoder) {
   Model *modelMentalBinaryDecoder = Model::create<MentalBinaryDecoder, MentalBinaryDecoderWidget>("mental", "MentalBinaryDecoder", "Binary Decoder", UTILITY_TAG);
   return modelMentalBinaryDecoder;
}
