///////////////////////////////////////////////////
//
//   Clock Divider VCV Module
//   Based on autodafe's clock divider but code cleaned up and extra outputs added.
//
//   Strum 2017
//
///////////////////////////////////////////////////

#include "mental.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_mental {

////////////////////////////////////////////////
struct MentalClockDivider : Module {
	enum ParamIds {
		RESET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1,
		OUT2,
		OUT4,
		OUT8,
		OUT16,
		OUT32,
    OUT3,
		OUT5,
		OUT7,
		OUT12,
		NUM_OUTPUTS
	};  
  enum LightIds {
		LIGHTS,
		NUM_LIGHTS = LIGHTS + 10
	};
  
	int clock2Count = 0;
	int clock4Count = 0;
	int clock8Count = 0;
	int clock16Count = 0;
	int clock32Count = 0;
  
  int clock3Count = 0;
  int clock5Count = 0;
  int clock7Count = 0;
  int clock12Count = 0;

	SchmittTrigger trigger2;
	SchmittTrigger trigger4;
	SchmittTrigger trigger8;
	SchmittTrigger trigger16;
	SchmittTrigger trigger32;
  
  SchmittTrigger trigger3;
  SchmittTrigger trigger5;
  SchmittTrigger trigger7;
  SchmittTrigger trigger12;

	SchmittTrigger reset_trig;


	MentalClockDivider();
	void step() override;
};

////////////////////////////////////////////////////////////////

MentalClockDivider::MentalClockDivider() {
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);
  lights.resize(NUM_LIGHTS);
	
}

int divider2 = 2;
int divider4 = 4;
int divider8 = 8;
int divider16 = 16;
int divider32 = 32;

int divider3 = 3;
int divider5 = 5;
int divider7 = 7;
int divider12 = 12;

//////////////////////////////////////////////////////////////////////////////////
void MentalClockDivider::step()
{
	bool reset = false;

	if (reset_trig.process(params[RESET_PARAM].value))
	{
		clock2Count = 0;
		clock4Count = 0;
		clock8Count = 0;
		clock16Count = 0;
		clock32Count = 0;

    clock3Count = 0;
    clock5Count = 0;
    clock7Count = 0;
    clock12Count = 0;

		reset = true;
	}
  
	if (clock2Count >= divider2)
	{
		clock2Count = 0;		
		reset = true;
	}
	if (clock4Count >= divider4)
	{
		clock4Count = 0;		
		reset = true;
	}
	if (clock8Count >= divider8)
	{
		clock8Count = 0;		
		reset = true;
	}
	if (clock16Count >= divider16)
	{
		clock16Count = 0;		
		reset = true;
	}
	if (clock32Count >= divider32)
	{
		clock32Count = 0;	
		reset = true;
	}  
  if (clock3Count >= divider3)
	{
		clock3Count = 0;		
		reset = true;
	}
  if (clock5Count >= divider5)
	{
		clock5Count = 0;		
		reset = true;
	}
  if (clock7Count >= divider7)
	{
		clock7Count = 0;		
		reset = true;
	}
  if (clock12Count >= divider12)
	{
		clock12Count = 0;		
		reset = true;
	}
  
/////////////////////////////////////////////////////////////
	if (clock2Count < divider2 / 2)
	{
		outputs[OUT2].value= 10.0;
		if (clock2Count == 0)
		{
      lights[LIGHTS].value = 1.0;
		}
		else
		{
       // sample rate has changed, update this.
			//lights[0] -= lights[0] / lightLambda / gSampleRate;
      //lights[0] -= lights[0] / lightLambda / engineGetSampleRate();
      lights[LIGHTS].value = 0.0;
		}		
	}
	else
	{
		outputs[OUT2].value= 0.0;
    lights[LIGHTS].value = 0.0;		
	}

	if (clock4Count < divider4 / 2)
	{
		outputs[OUT4].value= 10.0;
		if (clock4Count == 0)
		{
      lights[LIGHTS + 1].value = 1.0;
		}
		else
		{
      lights[LIGHTS + 1].value = 0.0;
		}		
	}
	else
	{
		outputs[OUT4].value= 0.0;
    lights[LIGHTS + 1].value = 0.0;		
	}
  
	if (clock8Count < divider8 / 2)
	{
		outputs[OUT8].value= 10.0;
		if (clock8Count == 0)
		{
      lights[LIGHTS + 2].value = 1.0;
		}
		else
		{
      lights[LIGHTS + 2].value = 0.0;
		}	
	}
	else
	{
		outputs[OUT8].value= 0.0;
    lights[LIGHTS + 2].value = 1.0;		
	}

	if (clock16Count < divider16 / 2)
	{
		outputs[OUT16].value= 10.0;
		if (clock16Count == 0)
		{
      lights[LIGHTS + 3].value = 1.0;
		}
		else
		{
      lights[LIGHTS + 3].value = 0.0;
		}		
	}
	else
	{
		outputs[OUT16].value= 0.0;
    lights[LIGHTS + 3].value = 0.0;	
	}

	if (clock32Count < divider32 / 2)
	{
		outputs[OUT32].value= 10.0;
		if (clock16Count == 0)
		{
      lights[LIGHTS + 4].value = 1.0;
		}
		else
		{
      lights[LIGHTS + 4].value = 0.0;
		}		
	}
	else
	{
		outputs[OUT32].value= 0.0;
    lights[LIGHTS + 4].value = 0.0;
	}
  
  if (clock3Count < divider3 / 2) outputs[OUT3].value = 10.0;	else outputs[OUT3].value = 0.0;
  if (clock5Count < divider5 / 2) outputs[OUT5].value = 10.0;	else outputs[OUT5].value = 0.0;
  if (clock7Count < divider7 / 2) outputs[OUT7].value = 10.0;	else outputs[OUT7].value = 0.0;
  if (clock12Count <divider12/2) outputs[OUT12].value = 10.0; else outputs[OUT12].value = 0.0;
  
//////////////////////////////////////////////////////////////////////////////
	if (reset == false)
	{
		if (trigger2.process(inputs[CLOCK_INPUT].value) && clock2Count <= divider2)
					clock2Count++;			
		if (trigger4.process(inputs[CLOCK_INPUT].value) && clock4Count <= divider4)
					clock4Count++;			
		if (trigger8.process(inputs[CLOCK_INPUT].value) && clock8Count <= divider8)
					clock8Count++;			
		if (trigger16.process(inputs[CLOCK_INPUT].value) && clock16Count <= divider16)		
			clock16Count++;			
		if (trigger32.process(inputs[CLOCK_INPUT].value) && clock32Count <= divider32)		
			clock32Count++;	
    if (trigger3.process(inputs[CLOCK_INPUT].value) && clock3Count <= divider3)		
			clock3Count++;
    if (trigger5.process(inputs[CLOCK_INPUT].value) && clock5Count <= divider5)		
			clock5Count++;
    if (trigger7.process(inputs[CLOCK_INPUT].value) && clock7Count <= divider7)		
			clock7Count++;
    if (trigger12.process(inputs[CLOCK_INPUT].value) && clock12Count <= divider12)		
			clock12Count++;	
	}
}

/////////////////////////////////////////////////////////////////////////////////////
struct MentalClockDividerWidget : ModuleWidget {
	MentalClockDividerWidget(MentalClockDivider *module);	
};

MentalClockDividerWidget::MentalClockDividerWidget(MentalClockDivider *module) : ModuleWidget(module)
{

	setPanel(SVG::load(assetPlugin(plugin, "res/MentalClockDivider.svg")));

	addInput(Port::create<GateInPort>(Vec(3, 20), Port::INPUT, module, MentalClockDivider::CLOCK_INPUT));
	addInput(Port::create<GateInPort>(Vec(3, 55), Port::INPUT, module, MentalClockDivider::RESET_INPUT));
	addParam(ParamWidget::create<LEDButton>(Vec(5, 80), module, MentalClockDivider::RESET_PARAM, 0.0, 1.0, 0.0));
	 
	addOutput(Port::create<GateOutPort>(Vec(2, 120), Port::OUTPUT, module, MentalClockDivider::OUT2));  
	addOutput(Port::create<GateOutPort>(Vec(2, 145), Port::OUTPUT, module, MentalClockDivider::OUT4));  
	addOutput(Port::create<GateOutPort>(Vec(2, 170), Port::OUTPUT, module, MentalClockDivider::OUT8));  
	addOutput(Port::create<GateOutPort>(Vec(2, 195), Port::OUTPUT, module, MentalClockDivider::OUT16));
	addOutput(Port::create<GateOutPort>(Vec(2, 220), Port::OUTPUT, module, MentalClockDivider::OUT32));
  
  addOutput(Port::create<GateOutPort>(Vec(2, 250), Port::OUTPUT, module, MentalClockDivider::OUT3));
  addOutput(Port::create<GateOutPort>(Vec(2, 275), Port::OUTPUT, module, MentalClockDivider::OUT5));
  addOutput(Port::create<GateOutPort>(Vec(2, 300), Port::OUTPUT, module, MentalClockDivider::OUT7));
  addOutput(Port::create<GateOutPort>(Vec(2, 325), Port::OUTPUT, module, MentalClockDivider::OUT12));
  
	
	addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(33, 120), module, MentalClockDivider::LIGHTS));
	addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(33, 145), module, MentalClockDivider::LIGHTS+1));
	addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(33, 170), module, MentalClockDivider::LIGHTS+2));
	addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(33, 195), module, MentalClockDivider::LIGHTS+3));  
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(33, 220), module, MentalClockDivider::LIGHTS+4));
  
	addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(33, 255), module, MentalClockDivider::LIGHTS+5));
	addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(33, 275), module, MentalClockDivider::LIGHTS+6));
	addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(33, 305), module, MentalClockDivider::LIGHTS+7));
  addChild(ModuleLightWidget::create<MedLight<BlueLED>>(Vec(33, 330), module, MentalClockDivider::LIGHTS+8));
	

}

} // namespace rack_plugin_mental

using namespace rack_plugin_mental;

RACK_PLUGIN_MODEL_INIT(mental, MentalClockDivider) {
   Model *modelMentalClockDivider = Model::create<MentalClockDivider, MentalClockDividerWidget>("mental", "MentalClockDivider", "Clock Divider", UTILITY_TAG);
   return modelMentalClockDivider;
}
