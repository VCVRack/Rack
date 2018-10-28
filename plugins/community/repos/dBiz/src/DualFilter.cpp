#include "dBiz.hpp"
#include "dsp/decimator.hpp"

using namespace std;

namespace rack_plugin_dBiz {

#define pi 3.14159265359

struct MultiFilter
{
	float q;
	float freq;
	float smpRate;
	float hp = 0.0f,bp = 0.0f,lp = 0.0f,mem1 = 0.0f,mem2 = 0.0f;

	void setParams(float freq, float q, float smpRate) {
		this->freq = freq;
		this->q=q;
		this->smpRate=smpRate;
	}

	void calcOutput(float sample)
	{
		float g = tan(pi*freq/smpRate);
		float R = 1.0f/(2.0f*q);
		hp = (sample - (2.0f*R + g)*mem1 - mem2)/(1.0f + 2.0f*R*g + g*g);
		bp = g*hp + mem1;
		lp = g*bp +  mem2;
		mem1 = g*hp + bp;
		mem2 = g*bp + lp;
	}

};

struct DualFilter : Module{
	enum ParamIds
	{
		CUTOFF_PARAM,
		Q_PARAM,
		CMOD_PARAM,
		CMOD_PARAM2,
		DRIVE_PARAM,
		CUTOFF2_PARAM,
		Q2_PARAM,
		CMOD2_PARAM,
		CMOD2_PARAM2,
		DRIVE2_PARAM,

		FADE_PARAM,

		VOLA_PARAM,
		VOLB_PARAM,

		FILTERSEL_PARAM,
		FILTER2SEL_PARAM,

		NUM_PARAMS
	};
	enum InputIds
	{
		CUTOFF_INPUT,
		CUTOFF_INPUT2,
		Q_INPUT,
		DRIVE_INPUT,
		IN,
		IN2,
		CUTOFF2_INPUT,
		CUTOFF2_INPUT2,
		Q2_INPUT,
		DRIVE2_INPUT,
		FADE_CV,

		NUM_INPUTS
	};
	enum OutputIds
	{
		OUT1,
		OUT2,
		MIXOUT,
		NUM_OUTPUTS
	};

	enum LightIds
	{
	  FADEA_LIGHTS,
	  FADEB_LIGHTS,
	  NUM_LIGHTS
	};
    

MultiFilter filterA;	// create a lpFilter;
MultiFilter filterB;	// create a lpFilter;

DualFilter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}
void step() override;

};

 float outLP;
 float outHP;
 float outBP;
 
 float out2LP;
 float out2HP;
 float out2BP;



void DualFilter::step() {

	float cutoff = pow(2.0f,rescale(clamp(params[CUTOFF_PARAM].value +quadraticBipolar(params[CMOD_PARAM2].value)*0.1f*inputs[CUTOFF_INPUT2].value+quadraticBipolar(params[CMOD_PARAM].value)*0.1f*inputs[CUTOFF_INPUT].value / 5.0f,0.0f,1.0f),0.0f,1.0f,4.5f,13.0f));
	float cutoff2 = pow(2.0f,rescale(clamp(params[CUTOFF2_PARAM].value +quadraticBipolar(params[CMOD2_PARAM2].value)*0.1f*inputs[CUTOFF2_INPUT2].value +quadraticBipolar(params[CMOD2_PARAM].value)*0.1f*inputs[CUTOFF2_INPUT].value / 5.0f,0.0f,1.0f),0.0f,1.0f,4.5f,13.0f));

	float q = 10.0f * clamp(params[Q_PARAM].value + inputs[Q_INPUT].value / 5.0f, 0.1f, 1.0f);
	float q2 = 10.0f * clamp(params[Q2_PARAM].value + inputs[Q2_INPUT].value / 5.0f, 0.1f, 1.0f);

	filterA.setParams(cutoff,q,engineGetSampleRate());
	filterB.setParams(cutoff2,q2,engineGetSampleRate());

	float in = inputs[IN].value * params[VOLA_PARAM].value / 5.0f;
	float in2 = inputs[IN2].value * params[VOLB_PARAM].value/ 5.0f;

////////////////////////////////////////////////////////////////


	in = clamp(in, -5.0f, 5.0f) * 0.2f;
	in2 = clamp(in2, -5.0f, 5.0f) * 0.2f;

	float a_shape = params[DRIVE_PARAM].value + clamp(inputs[DRIVE_INPUT].value, -5.0f, 5.0f);
	a_shape = clamp(a_shape, -5.0f, 5.0f) * 0.2f;
	a_shape *= 0.99f;

	float b_shape = params[DRIVE2_PARAM].value + clamp(inputs[DRIVE2_INPUT].value, -5.0f, 5.0f);
	b_shape = clamp(b_shape, -5.0f, 5.0f) * 0.2f;
	b_shape *= 0.99f;

	const float a_shapeB = (1.0 - a_shape) / (1.0 + a_shape);
	const float a_shapeA = (4.0 * a_shape) / ((1.0 - a_shape) * (1.0 + a_shape));

	const float b_shapeB = (1.0 - b_shape) / (1.0 + b_shape);
	const float b_shapeA = (4.0 * b_shape) / ((1.0 - b_shape) * (1.0 + b_shape));

	float a_outputd = in * (a_shapeA + a_shapeB);
	float b_outputd = in2 * (b_shapeA + b_shapeB);

	a_outputd = a_outputd / ((std::abs(in) * a_shapeA) + a_shapeB);
	b_outputd = b_outputd / ((std::abs(in2) * b_shapeA) + b_shapeB);

///////////////////////////////////////////////////////////////////


	filterA.calcOutput(a_outputd);
	filterB.calcOutput(b_outputd);

	float xfade = params[FADE_PARAM].value+inputs[FADE_CV].value / 10.0;
	lights[FADEA_LIGHTS].value=(1-xfade);
	lights[FADEB_LIGHTS].value=xfade;


	int sel1 = round(params[FILTERSEL_PARAM].value);
	int sel2 = round(params[FILTER2SEL_PARAM].value);

	for (int i=0;i<4;i++)
	{
	if (sel1 == 0) 
		outputs[OUT1].value = filterA.lp * 3.0f;
	if (sel1 == 1)
		outputs[OUT1].value = filterA.bp * 3.0f;
	if (sel1 == 2)
		outputs[OUT1].value = filterA.hp * 3.0f;
    }


	for (int i = 0; i < 4; i++)
	{
		if (sel2 == 0)
			outputs[OUT2].value = filterB.lp * 3.0f;
		if (sel2 == 1)
			outputs[OUT2].value = filterB.bp * 3.0f;
		if (sel2 == 2)
			outputs[OUT2].value = filterB.hp * 3.0f;
    }

	float filter1 = outputs[OUT1].value;
	float filter2 = outputs[OUT2].value;


	outputs[MIXOUT].value = (filter1 * ( 1-xfade ))+(filter2 * xfade);

    }
    


struct DualFilterWidget:ModuleWidget {
    DualFilterWidget(DualFilter *module) : ModuleWidget(module)
    {
        box.size = Vec(15*16, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/DualFilter.svg")));
		addChild(panel);
	}
    addChild(Widget::create<ScrewBlack>(Vec(15, 0)));
  	addChild(Widget::create<ScrewBlack>(Vec(box.size.x-30, 0)));
  	addChild(Widget::create<ScrewBlack>(Vec(15, 365)));
  	addChild(Widget::create<ScrewBlack>(Vec(box.size.x-30, 365)));


int i=120;
int s=27;
int l=7;
int of = -25;
int cv = 310;

 
	addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(i-40,21),module,DualFilter::FADEA_LIGHTS));
	addChild(GrayModuleLightWidget::create<MediumLight<GreenLight>>(Vec(i+28,21),module,DualFilter::FADEB_LIGHTS));

	addParam(ParamWidget::create<LRoundBlu>(Vec(i -22 , 21), module, DualFilter::FADE_PARAM, 0.0,1.0,0.0));


	addParam(ParamWidget::create<LRoundWhy>(Vec(35, 30), module, DualFilter::CUTOFF_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<LRoundWhy>(Vec(160, 30), module, DualFilter::CUTOFF2_PARAM, 0.0f, 1.0f, 1.0f));

	addParam(ParamWidget::create<RoundWhy>(Vec(of + 33, 113), module, DualFilter::CMOD_PARAM, -1.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundWhy>(Vec(of + 90, 93), module, DualFilter::Q_PARAM, 0.1f, 1.0f, 0.1f));

	addParam(ParamWidget::create<RoundWhy>(Vec(of + 95 + i, 113), module, DualFilter::CMOD2_PARAM, -1.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundWhy>(Vec(of + 39 + i, 93), module, DualFilter::Q2_PARAM, 0.1f, 1.0f, 0.1f));

	addParam(ParamWidget::create<RoundWhy>(Vec(of + 33, 165), module, DualFilter::CMOD_PARAM2, -1.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundWhy>(Vec(of + 95 + i, 165), module, DualFilter::CMOD2_PARAM2, -1.0, 1.0, 0.0));

	addParam(ParamWidget::create<RoundRed>(Vec(of + 90, 145), module, DualFilter::DRIVE_PARAM, -5.0f, 5.0f, 0.0f));
	addParam(ParamWidget::create<RoundRed>(Vec(of + 39 + i, 145), module, DualFilter::DRIVE2_PARAM, -5.0f, 5.0f, 0.0f));

	addParam(ParamWidget::create<RoundWhy>(Vec(of + 90, 200), module, DualFilter::VOLA_PARAM, 0.0, 5.0, 0.0));
	addParam(ParamWidget::create<RoundWhy>(Vec(of + 39 + i, 200), module, DualFilter::VOLB_PARAM, 0.0, 5.0, 0.0));


	addParam(ParamWidget::create<RoundWhySnapKnob>(Vec(of + 33, 220), module, DualFilter::FILTERSEL_PARAM, 0.0, 2.0, 0.0));
	addParam(ParamWidget::create<RoundWhySnapKnob>(Vec(of + 95 + i, 220), module, DualFilter::FILTER2SEL_PARAM, 0.0, 2.0, 0.0));




	addInput(Port::create<PJ301MCPort>(Vec(l, 276),Port::INPUT, module, DualFilter::CUTOFF_INPUT));
	addInput(Port::create<PJ301MCPort>(Vec(l + s , 276),Port::INPUT, module, DualFilter::CUTOFF_INPUT2));
	addInput(Port::create<PJ301MCPort>(Vec(l + s * 2, 276),Port::INPUT, module, DualFilter::Q_INPUT));
	addInput(Port::create<PJ301MOrPort>(Vec(l + s * 3, 276),Port::INPUT, module, DualFilter::DRIVE_INPUT));

	addInput(Port::create<PJ301MCPort>(Vec(l + s , cv),Port::INPUT, module, DualFilter::FADE_CV));
 
	addInput(Port::create<PJ301MCPort>(Vec(l+s*3 + i, 276),Port::INPUT, module, DualFilter::CUTOFF2_INPUT));
	addInput(Port::create<PJ301MCPort>(Vec(l + s*2 + i, 276),Port::INPUT, module, DualFilter::CUTOFF2_INPUT2));
	addInput(Port::create<PJ301MCPort>(Vec(l + s + i, 276),Port::INPUT, module, DualFilter::Q2_INPUT));
	addInput(Port::create<PJ301MOrPort>(Vec(l + i, 276),Port::INPUT, module, DualFilter::DRIVE2_INPUT));

	addInput(Port::create<PJ301MIPort>(Vec(l + s * 2, cv),Port::INPUT, module, DualFilter::IN));
	addInput(Port::create<PJ301MIPort>(Vec(l + s  + i, cv),Port::INPUT, module, DualFilter::IN2));

	addOutput(Port::create<PJ301MOPort>(Vec(l , cv),Port::OUTPUT, module, DualFilter::OUT1));

	addOutput(Port::create<PJ301MOPort>(Vec(l + s * 3 + i, cv),Port::OUTPUT, module, DualFilter::OUT2));

	addOutput(Port::create<PJ301MOPort>(Vec(l + s * 2 + i, cv),Port::OUTPUT, module, DualFilter::MIXOUT));
}
}; 
} // namespace rack_plugin_dBiz

using namespace rack_plugin_dBiz;

RACK_PLUGIN_MODEL_INIT(dBiz, DualFilter) {
   Model *modelDualFilter = Model::create<DualFilter, DualFilterWidget>("dBiz", "DualFilter", "Dual Multimode Filter", FILTER_TAG);
   return modelDualFilter;
}
