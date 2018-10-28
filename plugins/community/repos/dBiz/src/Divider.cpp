///////////////////////////////////////////////////
//  dBiz Divider
// 
///////////////////////////////////////////////////

#include "dBiz.hpp"
#include "dsp/digital.hpp"

using namespace std;

namespace rack_plugin_dBiz {

/////added fine out /////////////////////////////////////////////////
struct Divider : Module {
  enum ParamIds
  {
		MODE_PARAM,
    DIVISION_PARAM=MODE_PARAM+2,
    DIVISIONB_PARAM = DIVISION_PARAM +4,
    ON_SWITCH = DIVISIONB_PARAM +4,
    ON_SWITCHB = ON_SWITCH+4,
    NUM_PARAMS = ON_SWITCHB+4,
  };
  enum InputIds {
    CLOCK_INPUT,
    CLOCKB_INPUT,
		SUB1_INPUT,
		SUB2_INPUT=SUB1_INPUT+4,
    NUM_INPUTS=SUB2_INPUT+4
	};
	enum OutputIds
	{
		TRIG_OUTPUT,
		AB_OUTPUT,
		CD_OUTPUT,
		TRIGB_OUTPUT,
		AB2_OUTPUT,
		CD2_OUTPUT,
		NUM_OUTPUTS
	};

	enum LighIds
	{
		LIGHT_S1,
		LIGHT_S2 = LIGHT_S1 + 4,
		NUM_LIGHTS = LIGHT_S2 + 4
	};


	int clock1Count = 0;
	int clock2Count = 0;
	int clock3Count = 0;
	int clock4Count = 0;

	int clock1bCount = 0;
	int clock2bCount = 0;
	int clock3bCount = 0;
	int clock4bCount = 0;

  PulseGenerator clk1;
  PulseGenerator clk2;
  PulseGenerator clk3;
  PulseGenerator clk4;

	PulseGenerator clk1b;
  PulseGenerator clk2b;
  PulseGenerator clk3b;
  PulseGenerator clk4b;


	bool pulse1 = false;
	bool pulse2 = false;
	bool pulse3 = false;
	bool pulse4 = false;

	bool pulse1b = false;
	bool pulse2b = false;
	bool pulse3b = false;
	bool pulse4b = false;

	SchmittTrigger clk;
	SchmittTrigger clkb;

  Divider() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

  void step() override;
  
};

int divider1 = 0;
int divider2 = 0;
int divider3 = 0;
int divider4 = 0;

int divider1b = 0;
int divider2b = 0;
int divider3b = 0;
int divider4b = 0;
 
/////////////////////////////////////////////////////
void Divider::step() {



divider1 = round(params[DIVISION_PARAM].value   + clamp(inputs[SUB1_INPUT+0].value, -15.0f, 15.0f));
if (divider1>15) divider1=15; 
if (divider1<=1) divider1=1;
divider2 = round(params[DIVISION_PARAM+1].value + clamp(inputs[SUB1_INPUT+1].value, -15.0f, 15.0f));
if (divider2>15) divider2=15; 
if (divider2<=1) divider2=1;
divider3 = round(params[DIVISION_PARAM+2].value + clamp(inputs[SUB1_INPUT+2].value, -15.0f, 15.0f));
if (divider3>15) divider3=15; 
if (divider3<=1) divider3=1;
divider4 = round(params[DIVISION_PARAM+3].value + clamp(inputs[SUB1_INPUT+3].value, -15.0f, 15.0f));
if (divider4>15) divider4=15; 
if (divider4<=1) divider4=1;

divider1b = round(params[DIVISIONB_PARAM].value   + clamp(inputs[SUB2_INPUT+0].value, -15.0f, 15.0f));
if (divider1b>15) divider1b=15; 
if (divider1b<=1) divider1b=1;
divider2b = round(params[DIVISIONB_PARAM+1].value + clamp(inputs[SUB2_INPUT+1].value, -15.0f, 15.0f));
if (divider2b>15) divider2b=15; 
if (divider2b<=1) divider2b=1;
divider3b = round(params[DIVISIONB_PARAM+2].value + clamp(inputs[SUB2_INPUT+2].value, -15.0f, 15.0f));
if (divider3b>15) divider3b=15; 
if (divider3b<=1) divider3b=1;
divider4b = round(params[DIVISIONB_PARAM+3].value + clamp(inputs[SUB2_INPUT+3].value, -15.0f, 15.0f));
if (divider4b>15) divider4b=15; 
if (divider4b<=1) divider4b=1;


if (clk.process(inputs[CLOCK_INPUT].value))
 {
		clock1Count++;
		clock2Count++;		
		clock3Count++;		
		clock4Count++;					
 }

 if (clkb.process(inputs[CLOCKB_INPUT].value))
 {
		clock1bCount++;
		clock2bCount++;		
		clock3bCount++;		
		clock4bCount++;					
 }

if (clock1Count == 0) lights[LIGHT_S1+0].value = 1.0f; else lights[LIGHT_S1+0].value = 0.0;
if (clock2Count == 0) lights[LIGHT_S1+1].value = 1.0f; else lights[LIGHT_S1+1].value = 0.0;
if (clock3Count == 0) lights[LIGHT_S1+2].value = 1.0f; else lights[LIGHT_S1+2].value = 0.0;
if (clock4Count == 0) lights[LIGHT_S1+3].value = 1.0f; else lights[LIGHT_S1+3].value = 0.0;

if (clock1bCount == 0) lights[LIGHT_S2+0].value = 1.0f; else lights[LIGHT_S2+0].value = 0.0;
if (clock2bCount == 0) lights[LIGHT_S2+1].value = 1.0f; else lights[LIGHT_S2+1].value = 0.0;
if (clock3bCount == 0) lights[LIGHT_S2+2].value = 1.0f; else lights[LIGHT_S2+2].value = 0.0;
if (clock4bCount == 0) lights[LIGHT_S2+3].value = 1.0f; else lights[LIGHT_S2+3].value = 0.0;
  
	
	/////////////////////////////////////////////////////////////////

if(params[ON_SWITCH+0].value)
{
	if (clock1Count >= divider1)
	{
		clock1Count = 0;		
	  clk1.trigger(1e-3);  
	}
}
if(params[ON_SWITCH+1].value)
{
	if (clock2Count >= divider2)
	{
		clock2Count = 0;		
	  clk2.trigger(1e-3);
  }
}
if(params[ON_SWITCH+2].value)
{
	if (clock3Count >= divider3)
	{
		clock3Count = 0;		
	  clk3.trigger(1e-3);
  }
}
if(params[ON_SWITCH+3].value)
{
	if (clock4Count >= divider4)
	{
		clock4Count = 0;		
	  clk4.trigger(1e-3);
  }
} 	


if(params[ON_SWITCHB+0].value)
{
	if (clock1bCount >= divider1b)
	{
		clock1bCount = 0;		
	  clk1b.trigger(1e-3);  
	}
}
if(params[ON_SWITCHB+1].value)
{
	if (clock2bCount >= divider2b)
	{
		clock2bCount = 0;		
	  clk2b.trigger(1e-3);
  }
}
if(params[ON_SWITCHB+2].value)
{
	if (clock3bCount >= divider3b)
	{
		clock3bCount = 0;		
	  clk3b.trigger(1e-3);
  }
}
if(params[ON_SWITCHB+3].value)
{
	if (clock4bCount >= divider4b)
	{
		clock4bCount = 0;		
	  clk4b.trigger(1e-3);
  }
} 	

//////////////////////////////////////////////////////////////////
pulse1 = clk1.process(1.0f / engineGetSampleRate());
pulse2 = clk2.process(1.0f / engineGetSampleRate());
pulse3 = clk3.process(1.0f / engineGetSampleRate());
pulse4 = clk4.process(1.0f / engineGetSampleRate());

pulse1b = clk1b.process(1.0f / engineGetSampleRate());
pulse2b = clk2b.process(1.0f / engineGetSampleRate());
pulse3b = clk3b.process(1.0f / engineGetSampleRate());
pulse4b = clk4b.process(1.0f / engineGetSampleRate());

//////////////////////////////////////////////////////////////////
if(params[MODE_PARAM].value)
{
outputs[TRIG_OUTPUT].value =(((pulse1||pulse2)||pulse3)||pulse4)? 10.0f : 0.0f;
outputs[AB_OUTPUT].value = (pulse1 || pulse2) ? 10.0f : 0.0f;
outputs[CD_OUTPUT].value = (pulse3 || pulse4) ? 10.0f : 0.0f;
}
else
{
bool xora,xorb = false; 
xora = pulse1==pulse2;
xorb = pulse3==pulse4;
// outputs[TRIG_OUTPUT].value =(!pulse1 && (pulse2 ^ pulse3)) || (pulse1 && !(pulse2 || pulse3)) || (!pulse2 && (pulse3 ^ pulse4)) || (pulse2 && !(pulse3 || pulse4))? 10.0f : 0.0f;
outputs[TRIG_OUTPUT].value = xora == xorb ? 0.0f : 10.0f;
outputs[AB_OUTPUT].value = xora ? 0.0f : 10.0f;
outputs[CD_OUTPUT].value = xorb ? 0.0f : 10.0f;
}

if(params[MODE_PARAM+1].value)
{
outputs[TRIGB_OUTPUT].value =(((pulse1b||pulse2b)||pulse3b)||pulse4b)? 10.0f : 0.0f;
outputs[AB2_OUTPUT].value = (pulse1b || pulse2b) ? 10.0f : 0.0f;
outputs[CD2_OUTPUT].value = (pulse3b || pulse4b) ? 10.0f : 0.0f;
}
else
{
	bool xora2, xorb2 = false;
	xora2 = pulse1b == pulse2b;
	xorb2 = pulse3b == pulse4b;
	//outputs[TRIGB_OUTPUT].value = (!pulse1b && (pulse2b ^ pulse3b)) || (pulse1b && !(pulse2b || pulse3b)) || (!pulse2b && (pulse3b ^ pulse4b)) || (pulse2b && !(pulse3b || pulse4b)) ? 10.0f : 0.0f;
	outputs[TRIGB_OUTPUT].value = xora2 == xorb2 ? 0.0f : 10.0f;
	outputs[AB2_OUTPUT].value = xora2 ? 0.0f : 10.0f;
	outputs[CD2_OUTPUT].value = xorb2 ? 0.0f : 10.0f;
}
}

struct DividerWidget : ModuleWidget 
{
DividerWidget(Divider *module) : ModuleWidget(module)
{
	box.size = Vec(15*10, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin,"res/Divider.svg")));
		addChild(panel);
    }

//Screw
  addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
  addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));
    
   int knob=35;
   int jack = 27;
   int si = 15;   

   //
   for (int i = 0; i < 4; i++)
   {
	   addParam(ParamWidget::create<SDKnob>(Vec(si + 70, 20 + knob * i), module, Divider::DIVISION_PARAM + i, 1, 15, 1.0));
	   addParam(ParamWidget::create<SilverSwitch>(Vec(si + 10, 20 + knob * i), module, Divider::ON_SWITCH + i, 0.0, 1.0, 0.0));

	   addParam(ParamWidget::create<SDKnob>(Vec(si + 70, 170 + knob * i), module, Divider::DIVISIONB_PARAM + i, 1, 15, 1.0));
	   addParam(ParamWidget::create<SilverSwitch>(Vec(si + 10, 170 + knob * i), module, Divider::ON_SWITCHB + i, 0.0, 1.0, 0.0));

	   addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(si + 105, 30 + knob * i), module, Divider::LIGHT_S1 + i));
	   addChild(ModuleLightWidget::create<SmallLight<RedLight>>(Vec(si + 105, 180 + knob * i), module, Divider::LIGHT_S2 + i));

	   addInput(Port::create<PJ301MVAPort>(Vec(si + 40, 22.5 + knob * i), Port::INPUT, module, Divider::SUB1_INPUT + i));
	   addInput(Port::create<PJ301MVAPort>(Vec(si + 40, 173.5 + knob * i), Port::INPUT, module, Divider::SUB2_INPUT + i));
}


addInput(Port::create<PJ301MVAPort>(Vec(15, 310), Port::INPUT, module, Divider::CLOCK_INPUT));
addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 1, 310), Port::OUTPUT, module, Divider::AB_OUTPUT));
addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 2, 310), Port::OUTPUT, module, Divider::CD_OUTPUT));
addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 3, 310), Port::OUTPUT, module, Divider::TRIG_OUTPUT));
addParam(ParamWidget::create<MCKSSS>(Vec(15 + jack * 4, 313), module, Divider::MODE_PARAM + 0, 0.0, 1.0, 0.0));

addInput(Port::create<PJ301MVAPort>(Vec(15, 310 + jack), Port::INPUT, module, Divider::CLOCKB_INPUT));
addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 1, 310 + jack), Port::OUTPUT, module, Divider::AB2_OUTPUT));
addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 2, 310 + jack), Port::OUTPUT, module, Divider::CD2_OUTPUT));
addOutput(Port::create<PJ301MVAPort>(Vec(15 + jack * 3, 310 + jack), Port::OUTPUT, module, Divider::TRIGB_OUTPUT));

addParam(ParamWidget::create<MCKSSS>(Vec(15 + jack * 4, 313 + jack), module, Divider::MODE_PARAM + 1, 0.0, 1.0, 0.0));
}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, Divider) {
   Model *modelDivider = Model::create<Divider, DividerWidget>("dBiz", "Divider", "Divider", QUANTIZER_TAG);
   return modelDivider;
}
