
#include "dBiz.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_dBiz {

struct SmiX : Module {
	enum ParamIds
	{
		FALL_RANGE_PARAM,
		CLOCK_PARAM,
		OUT_A_ATT_PARAM,
		OUT_C_ATT_PARAM,
		MODE_BUTTON_PARAM,
		DIR_BUTTON_PARAM,
		MIX_SEL_PARAM,
		VOL_PARAM = MIX_SEL_PARAM + 8,
		NUM_PARAMS = VOL_PARAM + 8
	};
	enum InputIds
	{
		CLOCK_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		DIR_INPUT,
		MODE_INPUT,
		CV_INPUT,
		CH_INPUT = CV_INPUT + 8,
		NUM_INPUTS = CH_INPUT + 8
	};
	enum OutputIds {
		A_OUTPUT,
        B_OUTPUT,
        C_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds
	{
		CLOCK_LIGHT,
		CH_LIGHTS,
		MODE_LIGHTS = CH_LIGHTS + 8,
		DIR_LIGHTS = MODE_LIGHTS + 3,
		NUM_LIGHTS = DIR_LIGHTS + 4,
	};

	float ins[8] = {};
    float outs[3] = {};
	int mode = 0;
	int direction =0;
	bool dir=true;

	bool running = true;
	bool triggerActive = false;
	// for external clock
	SchmittTrigger clockTrigger;
	// For buttons
	SchmittTrigger resetTrigger;
	//SchmittTrigger gateTriggers[8];
	float phase = 0.0f;
	int index = 0;
	int stepIndex = index + 1;

	bool nextStep = false;
	bool gateState[8] = {};
	float resetLight = 0.0f;
	float stepLights[8] = {};
	const float lightLambda = 0.075f;

	SchmittTrigger mode_button_trigger;
	SchmittTrigger dir_button_trigger;

	// bool decaying = false;
	// float env = 0.0f;

	SmiX() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS,NUM_LIGHTS) {
	}
	void step() override;

	int numSteps;

	json_t *toJson() override
	{
		json_t *rootJ = json_object();
		// running
		json_object_set_new(rootJ, "running", json_boolean(running));
		// save mode
		json_t *modeJ = json_integer((int)mode);
		json_object_set_new(rootJ, "mode", modeJ);

		json_t *directionJ = json_integer((int)direction);
		json_object_set_new(rootJ, "direction", directionJ);

		return rootJ;

	}

	void fromJson(json_t *rootJ) override
	{
		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		json_t *modeJ = json_object_get(rootJ, "mode");
		if (modeJ)
		{
			mode = json_integer_value(modeJ);
		}

		json_t *directionJ = json_object_get(rootJ, "dir");
		if (directionJ)
		{
			direction = json_integer_value(directionJ);
		}

	}

	void reset() override
	{
		for (int i = 0; i < 8; i++)
		{
			gateState[i] = true;
		}
	}

};


void SmiX::step() {

	float lambda = 0.0f;
	switch ((int)params[FALL_RANGE_PARAM].value)
	{
	case 0:
		lambda = 0.005;
		break;
	case 1:
		lambda = 0.01;
		break;
	default:
		lambda = 0.03;
		break;
	}

	if(inputs[MODE_INPUT].active){
		mode = clamp(inputs[MODE_INPUT].value, 0.0f,2.0f);
		for (int i = 0; i < 3; i++)
		{
			lights[MODE_LIGHTS + i].value = 0.0;
		}
	}

	if (inputs[DIR_INPUT].active)
	{
		direction = clamp(inputs[DIR_INPUT].value, 0.0f, 3.0f);
		for (int i = 0; i < 4; i++)
		{
			lights[DIR_LIGHTS + i].value = 0.0;
		}
	}

	if (mode_button_trigger.process(params[MODE_BUTTON_PARAM].value))
	{
		mode++;
		if (mode > 2)
			mode = 0;
		for (int i = 0; i < 3; i++)
		{
			lights[MODE_LIGHTS + i].value = 0.0;
		}
	 }
	 lights[MODE_LIGHTS + mode].value = 1.0;

	 if (mode == 1 || mode == 2)
	 {
		 if (dir_button_trigger.process(params[DIR_BUTTON_PARAM].value))
		 {
			 direction++;
			 if (direction > 3)
				 direction = 0;
			 for (int i = 0; i < 4; i++)
			 {
				 lights[DIR_LIGHTS + i].value = 0.0;
			 }
		 }
		 lights[DIR_LIGHTS + direction].value = 1.0;
	 }

		 ///////////////////////////////////SEQ////////////////////////////////////////////////////

		 numSteps = 8;
		 stepIndex = index + 1;

		 // Run
		 if (mode == 1 || mode == 2)
		 {
			 running = !running;
		 }
		 else running = false;
 

	nextStep = false;

if (running)
{
	if (inputs[EXT_CLOCK_INPUT].active)
	{
		// External clock
		if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].value))
		{
			phase = 0.0f;
			nextStep = true;
		}
	}
	else
	{
		// Internal clock
		float clockTime = powf(2.0, params[CLOCK_PARAM].value + inputs[CLOCK_INPUT].value);
		phase += clockTime / engineGetSampleRate();
		if (phase >= 1.0f)
		{
			phase -= 1.0f;
			nextStep = true;
		}
	}
} 

// Reset
if (resetTrigger.process(inputs[RESET_INPUT].value))
{
	phase = 0.0f;
	index = 8;
	nextStep = true;
}

if (nextStep)
{
	// Advance step
	int numSteps = 8;

	switch ((int)direction)
	{
	case 0:
		index += 1;
		if (index >= numSteps)
		{
			index = 0;
		}
		break;
	case 1:
		index -= 1;
		if (index < 0)
		{
			index = 7;
		}
		break;
	case 2:
		if(dir)
		{
			index+=1;
			if (index >= numSteps-1)
			dir = false;
		}
		else{
			index -= 1;
			if (index < 1)
				dir=true;
		}
		break;
	default:
		index = randomUniform() * 8.0f;
		break;
	}

	stepLights[index] = 1.0f;
}



////////////////////////////////SEQ/////////////////


float a_out = 0.0f;
float b_out = 0.0f;
float c_out = 0.0f;

float sum = 0.0f;


for (int i = 0; i < 8; i++)
{
	 
	stepLights[i] -= stepLights[i] / lambda*lightLambda / engineGetSampleRate();
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (mode == 1 ){
		lights[CH_LIGHTS + i].value = stepLights[i] + clamp(params[VOL_PARAM + i].value, 0.0f, 1.0f) * clamp(inputs[CV_INPUT + i].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
		ins[i] = (inputs[CH_INPUT + i].value * params[VOL_PARAM + i].value / 2.0f) * clamp(inputs[CV_INPUT + i].normalize(10.0f) / 10.0f, 0.0f, 1.0f) + (inputs[CH_INPUT + i].value * params[VOL_PARAM + i].value * stepLights[i]*2.0f);
	}
	else 
	if ( mode == 2){
		lights[CH_LIGHTS + i].value = stepLights[i] * clamp(inputs[CV_INPUT + i].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
		ins[i] = 2.0f*inputs[CH_INPUT + i].value * params[VOL_PARAM + i].value * clamp(inputs[CV_INPUT + i].normalize(10.0f) / 10.0f, 0.0f, 1.0f) * stepLights[i];
	}
	else
	{
		lights[CH_LIGHTS + i].value = params[VOL_PARAM + i].value* clamp(inputs[CV_INPUT+i].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
		ins[i] = inputs[CH_INPUT + i].value * params[VOL_PARAM + i].value * clamp(inputs[CV_INPUT+i].normalize(10.0f) / 10.0f, 0.0f, 1.0f);
	}

	 switch ((int)params[MIX_SEL_PARAM + i].value)
	 {
	 case 0:
		 a_out += ins[i] / 1.3f;
		 break;
	 case 1:
		 b_out += ins[i] / 1.3f;
		 break;
	 default:
		 c_out += ins[i] / 1.3f;
		 break;
	}
	sum+=ins[i]/ 1.3f;
}

	outputs[A_OUTPUT].value = a_out * params[OUT_A_ATT_PARAM].value;
	outputs[B_OUTPUT].value = b_out;
	outputs[C_OUTPUT].value = c_out * params[OUT_C_ATT_PARAM].value;

	///////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

struct SmiXWidget : ModuleWidget 
{
SmiXWidget(SmiX *module) : ModuleWidget(module)
{
	box.size = Vec(15*16, 380);
	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/SmiX.svg")));
		addChild(panel);
	}
	addParam(ParamWidget::create<Trimpot>(mm2px(Vec(1.003 + 6.6 * 10.10, 55)), module, SmiX::CLOCK_PARAM, -2.0f, 6.0f, 2.0f));
	addParam(ParamWidget::create<MCKSSS>(mm2px(Vec(1.003 + 6.7 * 10.10, 62)), module, SmiX::FALL_RANGE_PARAM, 0.0, 2.0, 0.0));

	addChild(Widget::create<ScrewBlack>(Vec(15, 0)));
  	addChild(Widget::create<ScrewBlack>(Vec(box.size.x-30, 0)));
  	addChild(Widget::create<ScrewBlack>(Vec(15, 365)));
  	addChild(Widget::create<ScrewBlack>(Vec(box.size.x-30, 365)));

	 int x_offset = 10.10;
	 int y_offset = 2.0;
	 
	 for (int i = 0; i < 8; i++)
	 {
		 addParam(ParamWidget::create<LEDSliderBlue>(mm2px(Vec(2.792 + i * x_offset, (y_offset*3) + 3.937)), module, SmiX::VOL_PARAM + i, 0.0, 1.0, 0.0));

		 addInput(Port::create<PJ301MOrPort>(mm2px(Vec(1.003 + i * x_offset,y_offset+ 72.858)), Port::INPUT, module, SmiX::CV_INPUT + i));
		 addParam(ParamWidget::create<MCKSSS>(mm2px(Vec(4.003 + i * x_offset,y_offset+ 85)), module, SmiX::MIX_SEL_PARAM + i, 0.0, 2.0, 0.0));
		 addInput(Port::create<PJ301MIPort>(mm2px(Vec(1.003 + i * x_offset,y_offset+ 94.858)), Port::INPUT, module, SmiX::CH_INPUT + i));

		 addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(Vec(13 + i * 30, y_offset + 115), module, SmiX::CH_LIGHTS + i));
	}

	addInput(Port::create<PJ301MCPort>(mm2px(Vec(1.003 + 2 * x_offset, 61.915)), Port::INPUT, module, SmiX::EXT_CLOCK_INPUT));
	addInput(Port::create<PJ301MCPort>(mm2px(Vec(1.003 + 3 * x_offset, 61.915)), Port::INPUT, module, SmiX::MODE_INPUT));
	addInput(Port::create<PJ301MRPort>(mm2px(Vec(1.003 + 4 * x_offset, 61.915)), Port::INPUT, module, SmiX::DIR_INPUT));
	addInput(Port::create<PJ301MCPort>(mm2px(Vec(1.003 + 5 * x_offset, 61.915)), Port::INPUT, module, SmiX::RESET_INPUT));
	
	for (int i = 0 ; i < 3 ; i++)
  {
    addChild(ModuleLightWidget::create<MediumLight<BlueLight>>(mm2px(Vec((2.905 + i * 8)+16 , 49.035)), module, SmiX::MODE_LIGHTS + i));
  }
  for (int i = 0 ; i < 4 ; i++)
  {
    addChild(ModuleLightWidget::create<MediumLight<RedLight>>(mm2px(Vec(43+(i*5.3), 49.035)), module, SmiX::DIR_LIGHTS + i));
  }

  addParam(ParamWidget::create<LEDButton>(Vec(27 + 50, 160), module, SmiX::MODE_BUTTON_PARAM, 0.0, 1.0, 0.0));
  addParam(ParamWidget::create<LEDButton>(Vec(97 + 50, 160), module, SmiX::DIR_BUTTON_PARAM, 0.0, 1.0, 0.0));

  addOutput(Port::create<PJ301MOPort>(mm2px(Vec(1.003 + 1 * x_offset, 115.169)), Port::OUTPUT, module, SmiX::A_OUTPUT));
  addOutput(Port::create<PJ301MOPort>(mm2px(Vec(1.003 + 3.5 * x_offset, 115.169)), Port::OUTPUT, module, SmiX::B_OUTPUT));
  addOutput(Port::create<PJ301MOPort>(mm2px(Vec(1.003 + 6 * x_offset, 115.169)), Port::OUTPUT, module, SmiX::C_OUTPUT));

  addParam(ParamWidget::create<Trimpot>(mm2px(Vec(1.003 + 0.2 * x_offset, 115.169)), module, SmiX::OUT_A_ATT_PARAM, 0.0, 1.0, 0.0));
  addParam(ParamWidget::create<Trimpot>(mm2px(Vec(1.003 + 7 * x_offset, 115.169)), module, SmiX::OUT_C_ATT_PARAM, 0.0, 1.0, 0.0));
}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, SmiX) {
   Model *modelSmiX = Model::create<SmiX, SmiXWidget>("dBiz", "SmiX", "SmiX", MIXER_TAG);
   return modelSmiX;
}
