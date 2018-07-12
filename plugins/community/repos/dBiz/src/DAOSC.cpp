#include "dBiz.hpp"
#include "dsp/functions.hpp"

namespace rack_plugin_dBiz {

struct sinebank {

	float deltaTime = 1.0 / engineGetSampleRate();
	float phase = 0.0;
	float freq;
	float pitch;
	float pitchSlew = 0.0;
	int pitchSlewIndex = 0;

	//void setPitch(float pitchKnob, float pitchCv)
	void setPitch(float pitchKnob, float pitchCv)
	{
		// Compute frequency
		pitch = pitchKnob;
		// Apply pitch slew
		const float pitchSlewAmount = 3.0;
		pitch += pitchSlew * pitchSlewAmount;
		pitch += pitchCv;
		// Note C3
		freq = 261.626 * powf(2.0, pitch / 12.0);
		// Accumulate the phase
		phase += freq * deltaTime;
		if (phase >= 1.0)
			phase -= 1.0;
	}
	
	void setFreq(float freq2)
	{
		// Accumulate the phase
		phase += freq2 * deltaTime;
		if (phase >= 1.0)
			phase -= 1.0;
	}

	float light()
	{
		return sinf(2 * M_PI * phase);
	}
};


///////// inspired to Tutorial Module!!

struct DAOSC : Module {
    enum ParamIds
    {
        A_PITCH_PARAM,
        A_FINE_PARAM,
        A_FOLD_PARAM,
        A_DRIVE_PARAM,
        //A_MODE_PARAM,
        A_SAW_PARAM,
        A_SQUARE_PARAM,
        A_FM_PARAM,

        B_PITCH_PARAM,
        B_FINE_PARAM,
        B_FOLD_PARAM,
        B_DRIVE_PARAM,
        //B_MODE_PARAM,
        B_SAW_PARAM,
        B_SQUARE_PARAM,
        B_FM_PARAM,

        
        NUM_PARAMS
    };
	enum InputIds
	{

		A_FM_INPUT,
		A_SAW_INPUT,
		A_SQUARE_INPUT,
		A_PITCH_INPUT,
		A_FOLD_INPUT,
		A_DRIVE_INPUT,
		//A_OFF_INPUT,

		B_FM_INPUT,
		B_SAW_INPUT,
		B_SQUARE_INPUT,
		B_PITCH_INPUT,
		B_DRIVE_INPUT,
		B_FOLD_INPUT,
		//B_OFF_INPUT,
		NUM_INPUTS
	};
	enum OutputIds
	{
		A_OUTPUT,
        B_OUTPUT,
        SUM_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float phase = 0.0;
	float blinkPhase = 0.0;

	sinebank osc_a;
	sinebank a_harmonic[20]={};
	sinebank a_harmonicq[20] = {};
	sinebank osc_b;
	sinebank b_harmonic[20] = {};
	sinebank b_harmonicq[20] = {};

	DAOSC() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

};

void DAOSC::step() {


	int a_harm = round(params[A_SAW_PARAM].value+clamp(inputs[A_SAW_INPUT].value, 0.0f, 19.0f));
	int a_harmq = round(params[A_SQUARE_PARAM].value+clamp(inputs[A_SQUARE_INPUT].value, 0.0f, 19.0f));
	int b_harm = round(params[B_SAW_PARAM].value + clamp(inputs[B_SAW_INPUT].value, 0.0f, 19.0f));
	int b_harmq = round(params[B_SQUARE_PARAM].value + clamp(inputs[B_SQUARE_INPUT].value, 0.0f, 19.0f));

	if(a_harm >20) a_harm = 20;
	if(a_harmq>20) a_harmq = 20;
	if(b_harm >20) b_harm = 20;
	if(b_harmq>20) b_harmq = 20;


	float a_harmsum = 0.0;
	float a_harmsumq = 0.0;
	float b_harmsum = 0.0;
	float b_harmsumq = 0.0;

	float a_pitchCv = 12.0 * inputs[A_PITCH_INPUT].value;
	float b_pitchCv = 12.0 * inputs[B_PITCH_INPUT].value;

	float a_pitchFine = 3.0 * quadraticBipolar(params[A_FINE_PARAM].value);
	float b_pitchFine = 3.0 * quadraticBipolar(params[B_FINE_PARAM].value);

	if (inputs[A_FM_INPUT].active)
	{
		a_pitchCv += quadraticBipolar(params[A_FM_PARAM].value) * 12.0 * inputs[A_FM_INPUT].value;
	}

	if (inputs[B_FM_INPUT].active)
	{
		b_pitchCv += quadraticBipolar(params[B_FM_PARAM].value) * 12.0 * inputs[B_FM_INPUT].value;
	}

	osc_a.setPitch(params[A_PITCH_PARAM].value, a_pitchFine + a_pitchCv);
	osc_b.setPitch(params[B_PITCH_PARAM].value, b_pitchFine + b_pitchCv);

	for (int i = 1; i < a_harm; i++)
	{
		a_harmonic[i].setFreq((i*2)*osc_a.freq);
		a_harmsum += a_harmonic[i].light()*3.0/i;
	}

	for (int i = 1; i < a_harmq; i++)
	{
		a_harmonicq[i].setFreq(((i * 2) + 1) * osc_a.freq);
		a_harmsumq += a_harmonicq[i].light() * 3.0/ i;
	}

	for (int i = 1; i < b_harm; i++)
	{
		b_harmonic[i].setFreq((i * 2) * osc_b.freq);
		b_harmsum += b_harmonic[i].light() * 3.0/ i;
	}

	for (int i = 1; i < b_harmq; i++)
	{
		b_harmonicq[i].setFreq(((i * 2) + 1) * osc_b.freq);
		b_harmsumq += b_harmonicq[i].light() * 3.0 / i;
    }

	//////////////// Contrast - Thx to  Michael Hetrick!!!

	////////////////A

	float a_inputf = 3.0 * osc_a.light() + a_harmsum + a_harmsumq;
	float b_inputf = 3.0 * osc_b.light() + b_harmsum + b_harmsumq;

	a_inputf = clamp(a_inputf, -6.0f, 6.0f) * 0.2f;
	b_inputf = clamp(b_inputf, -6.0f, 6.0f) * 0.2f;

	float a_contrast = params[A_FOLD_PARAM].value + clamp(inputs[A_FOLD_INPUT].value, 0.0f, 6.0f);
	float b_contrast = params[B_FOLD_PARAM].value + clamp(inputs[B_FOLD_INPUT].value, 0.0f, 6.0f);

	a_contrast = clamp(a_contrast, 0.0f, 6.0f) * 0.2f;
	b_contrast = clamp(b_contrast, 0.0f, 6.0f) * 0.2f;

	const float a_factor1 = a_inputf * 1.57143;
	const float a_factor2 = sinf(a_inputf * 6.28571) * a_contrast;

	const float b_factor1 = b_inputf * 1.57143;
	const float b_factor2 = sinf(b_inputf * 6.28571) * b_contrast;

	float a_outputf = sinf(a_factor1 + a_factor2);
	a_outputf *= 6.0f;

	float b_outputf = sinf(b_factor1 + b_factor2);
	b_outputf *= 6.0f;

	//////////////////////////Wave shape - Thx to  Michael Hetrick!!!

	float a_inputd = a_outputf;
	float b_inputd = b_outputf;

	a_inputd = clamp(a_inputd, -5.0f, 5.0f) * 0.2f;
	b_inputd = clamp(b_inputd, -5.0f, 5.0f) * 0.2f;

	float a_shape = params[A_DRIVE_PARAM].value + clamp(inputs[A_DRIVE_INPUT].value, -5.0f, 5.0f);
	a_shape = clamp(a_shape, -5.0f, 5.0f) * 0.2f;
	a_shape *= 0.99f;

	float b_shape = params[B_DRIVE_PARAM].value + clamp(inputs[B_DRIVE_INPUT].value, -5.0f, 5.0f);
	b_shape = clamp(b_shape, -5.0f, 5.0f) * 0.2f;
	b_shape *= 0.99f;

	const float a_shapeB = (1.0 - a_shape) / (1.0 + a_shape);
	const float a_shapeA = (4.0 * a_shape) / ((1.0 - a_shape) * (1.0 + a_shape));

	const float b_shapeB = (1.0 - b_shape) / (1.0 + b_shape);
	const float b_shapeA = (4.0 * b_shape) / ((1.0 - b_shape) * (1.0 + b_shape));

	float a_outputd = a_inputd * (a_shapeA + a_shapeB);
	float b_outputd = b_inputd * (b_shapeA + b_shapeB);

	a_outputd = a_outputd / ((std::abs(a_inputd) * a_shapeA) + a_shapeB);
	b_outputd = b_outputd / ((std::abs(b_inputd) * b_shapeA) + b_shapeB);

	b_outputd *= 1.0f;

	////////////////////////////////////////////////////////
	outputs[A_OUTPUT].value = 3.0 * a_outputd;
	outputs[B_OUTPUT].value = 3.0 * b_outputd;

	outputs[SUM_OUTPUT].value = 3.0 * (a_outputd + b_outputd) / 2;
}

struct DAOSCWidget : ModuleWidget 
{
DAOSCWidget(DAOSC *module) : ModuleWidget(module)
{
	box.size = Vec(13 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/DAOSC.svg")));
		addChild(panel);
	}

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));


int knob=42;
int jack=30;
float mid = 97.5;
int top = 20;
int down = 50;

// addParam(ParamWidget::create<CKSS>(Vec(off * 2 + 18, 30), module, DAOSC::A_MODE_PARAM, 0.0, 1.0, 0.0));
// addParam(ParamWidget::create<CKSS>(Vec(off * 3 + 18, 30), module, DAOSC::B_MODE_PARAM, 0.0, 1.0, 0.0));

addParam(ParamWidget::create<LRoundWhy>(Vec(box.size.x-mid-50, top), module, DAOSC::A_PITCH_PARAM, -54.0, 54.0, 0.0));
addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x-mid-knob*2 - 10, top), module, DAOSC::A_FINE_PARAM, -1.0, 1.0, 0.0));
addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x - mid - knob * 1 , top + knob + 35), module, DAOSC::A_FM_PARAM, 0.0, 1.0, 0.0));
addParam(ParamWidget::create<RoundAzz>(Vec(box.size.x - mid - knob * 2 - 5, top + knob + 5), module, DAOSC::A_FOLD_PARAM, 0.0, 5.0, 0.0));
addParam(ParamWidget::create<RoundRed>(Vec(box.size.x - mid - knob * 2 - 5, 125), module, DAOSC::A_DRIVE_PARAM, -5.0, 5.0, 0.0));
addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x-mid-knob, 157), module, DAOSC::A_SQUARE_PARAM, 1.0, 20.0, 1.0));
addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x-mid-knob*2, 177), module, DAOSC::A_SAW_PARAM, 1.0, 20.0, 1.0));

addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid-jack-5, 160+down), Port::INPUT, module, DAOSC::A_FM_INPUT));
addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid-jack-5, 190+down), Port::INPUT, module, DAOSC::A_PITCH_INPUT));
addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid-jack*2-5, 190+down), Port::INPUT, module, DAOSC::A_FOLD_INPUT));
addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid-jack*3-5, 190+down), Port::INPUT, module, DAOSC::A_DRIVE_INPUT));
addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid-jack*2-5, 230+down), Port::INPUT, module, DAOSC::A_SAW_INPUT));
addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid-jack*3-5, 230+down), Port::INPUT, module, DAOSC::A_SQUARE_INPUT));

addOutput(Port::create<PJ301MOPort>(Vec(box.size.x - mid-jack-5, 230+down), Port::OUTPUT, module, DAOSC::A_OUTPUT));

addParam(ParamWidget::create<LRoundWhy>(Vec(box.size.x-mid+5, top), module, DAOSC::B_PITCH_PARAM, -54.0, 54.0, 0.0));
addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x-mid+5+knob+10, top), module, DAOSC::B_FINE_PARAM, -1.0, 1.0, 0.0));
addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x - mid + 5, top + knob+35), module, DAOSC::B_FM_PARAM, 0.0, 1.0, 0.0));
addParam(ParamWidget::create<RoundAzz>(Vec(box.size.x - mid + 10 + knob, top + knob + 5), module, DAOSC::B_FOLD_PARAM, 0.0, 5.0, 0.0));
addParam(ParamWidget::create<RoundRed>(Vec(box.size.x - mid + 10 + knob, 125), module, DAOSC::B_DRIVE_PARAM, -5.0, 5.0, 0.0));
addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x-mid+5, 157), module, DAOSC::B_SQUARE_PARAM, 1.0, 20.0, 1.0));
addParam(ParamWidget::create<RoundWhy>(Vec(box.size.x-mid+5+knob, 177), module, DAOSC::B_SAW_PARAM, 1.0, 20.0, 1.0));

addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid+10, 160+down), Port::INPUT, module, DAOSC::B_FM_INPUT));
addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid+10, 190+down), Port::INPUT, module, DAOSC::B_PITCH_INPUT));
addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid+10+jack, 190+down), Port::INPUT, module, DAOSC::B_FOLD_INPUT));
addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid+10+jack*2, 190+down), Port::INPUT, module, DAOSC::B_DRIVE_INPUT));
addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid+10+jack, 230+down), Port::INPUT, module, DAOSC::B_SAW_INPUT));
addInput(Port::create<PJ301MIPort>(Vec(box.size.x-mid+10+jack*2, 230+down), Port::INPUT, module, DAOSC::B_SQUARE_INPUT));

addOutput(Port::create<PJ301MOPort>(Vec(box.size.x - mid+10, 230+down), Port::OUTPUT, module, DAOSC::B_OUTPUT));

addOutput(Port::create<PJ301MOPort>(Vec(box.size.x - mid-12.5, 265+down), Port::OUTPUT, module, DAOSC::SUM_OUTPUT));
}
};

} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, DAOSC) {
   Model *modelDAOSC = Model::create<DAOSC, DAOSCWidget>("dBiz", "DAOSC", "DAOSC", OSCILLATOR_TAG);
   return modelDAOSC;
}

