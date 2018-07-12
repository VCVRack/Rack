//**************************************************************************************
//Clock Divider Module for VCV Rack by Autodafe http://www.autodafe.net
//
//  Based on code created by Created by Nigel Redmon 
//  EarLevel Engineering: earlevel.com
//  Copyright 2012 Nigel Redmon
//  http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
//**************************************************************************************

#include "dBiz.hpp"
#include <stdlib.h>

namespace rack_plugin_dBiz {

struct DualFilter : Module{
	enum ParamIds
	{
		FREQ_PARAM,
		Q_PARAM,
		RES_PARAM,
		FREQ_CV_PARAM,
		FREQ_CV_PARAM2,
		DRIVE_PARAM,
		FREQ2_PARAM,
		Q2_PARAM,
		RES2_PARAM,
		FREQ2_CV_PARAM,
		FREQ2_CV_PARAM2,
		DRIVE2_PARAM,

		ROUTE_PARAM,
		FADE_PARAM,

		VOLA_PARAM,
		VOLB_PARAM,

		FILTERSEL_PARAM,
		FILTER2SEL_PARAM,

		NUM_PARAMS
	};
	enum InputIds
	{
		FREQ_INPUT,
		FREQ_INPUT2,
		RES_INPUT,
		DRIVE_INPUT,
		INPUT,
		FREQ2_INPUT,
		FREQ2_INPUT2,
		RES2_INPUT,
		DRIVE2_INPUT,
		INPUT2,
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


	DualFilter();
VAStateVariableFilter *lpFilter = new VAStateVariableFilter() ;	// create a lpFilter;
VAStateVariableFilter *hpFilter = new VAStateVariableFilter() ;	// create a lpFilter;
VAStateVariableFilter *bpFilter = new VAStateVariableFilter() ;	// create a lpFilter;
VAStateVariableFilter *npFilter = new VAStateVariableFilter() ;	// create a lpFilter;

VAStateVariableFilter *lp2Filter = new VAStateVariableFilter(); // create a lpFilter;
VAStateVariableFilter *hp2Filter = new VAStateVariableFilter(); // create a lpFilter;
VAStateVariableFilter *bp2Filter = new VAStateVariableFilter(); // create a lpFilter;
VAStateVariableFilter *np2Filter = new VAStateVariableFilter(); // create a lpFilter;

void step()override;
};


DualFilter::DualFilter() {
	params.resize(NUM_PARAMS);
	inputs.resize(NUM_INPUTS);
	outputs.resize(NUM_OUTPUTS);
	lights.resize(NUM_LIGHTS);
}

float outLP;
float outHP;
float outBP;
float outNP;

float out2LP;
float out2HP;
float out2BP;
float out2NP;

//VAStateVariableFilter *peakFilter = new VAStateVariableFilter();

float minfreq = 15.0;
float maxfreq = 12000;


void DualFilter::step() {
	
	

	float input = inputs[INPUT].value * params[VOLA_PARAM].value / 5.0;
	float input2 = inputs[INPUT2].value * params[VOLB_PARAM].value/ 5.0;
	float drive = params[DRIVE_PARAM].value + inputs[DRIVE_INPUT].value / 10.0;
	float drive2 = params[DRIVE2_PARAM].value + inputs[DRIVE2_INPUT].value / 10.0;
	float xfade = params[FADE_PARAM].value+inputs[FADE_CV].value / 10.0;
	float gain = powf(100.0, drive);
	float gain2 = powf(100.0, drive2);
	input *= gain;
	input2 *= gain2;

	lights[FADEA_LIGHTS].value=(1-xfade);
	lights[FADEB_LIGHTS].value=xfade;


	// Add -60dB noise to bootstrap self-oscillation
	input += 1.0e-6 * (2.0*randomf() - 1.0)*1000;
	input2 += 1.0e-6 * (2.0 * randomf() - 1.0) * 1000;

	// Set resonance
	float res = clampf(params[RES_PARAM].value + clampf(inputs[RES_INPUT].value, 0,1), 0,1);
	float res2 = clampf(params[RES2_PARAM].value + clampf(inputs[RES2_INPUT].value, 0, 1), 0, 1);
	//res = 5.5 * clampf(res, 0.0, 1.0);
	

	float cutoffcv =  400*params[FREQ_CV_PARAM].value * inputs[FREQ_INPUT].value+ 400*inputs[FREQ_INPUT2].value *params[FREQ_CV_PARAM2].value ;
	float cutoff2cv = 400 * params[FREQ2_CV_PARAM].value * inputs[FREQ2_INPUT].value + 400 * inputs[FREQ2_INPUT2].value * params[FREQ2_CV_PARAM2].value;

	float cutoff = params[FREQ_PARAM].value + cutoffcv;
	float cutoff2 = params[FREQ2_PARAM].value + cutoff2cv;

	cutoff = clampf(cutoff, minfreq, maxfreq);
	cutoff2 = clampf(cutoff2, minfreq, maxfreq);


	lpFilter->setFilterType(0);
	hpFilter->setFilterType(2);
	bpFilter->setFilterType(1);
	npFilter->setFilterType(5);

	lp2Filter->setFilterType(0);
	hp2Filter->setFilterType(2);
	bp2Filter->setFilterType(1);
	np2Filter->setFilterType(5);

	lpFilter->setCutoffFreq(cutoff);
	hpFilter->setCutoffFreq(cutoff); 
	bpFilter->setCutoffFreq(cutoff);
	npFilter->setCutoffFreq(cutoff);

	lp2Filter->setCutoffFreq(cutoff2);
	hp2Filter->setCutoffFreq(cutoff2);
	bp2Filter->setCutoffFreq(cutoff2);
	np2Filter->setCutoffFreq(cutoff2);

	lpFilter->setResonance(res);
	hpFilter->setResonance(res);
	bpFilter->setResonance(res);
	npFilter->setResonance(res);

	lp2Filter->setResonance(res2);
	hp2Filter->setResonance(res2);
	bp2Filter->setResonance(res2);
	np2Filter->setResonance(res2);

	lpFilter->setSampleRate(engineGetSampleRate());
	hpFilter->setSampleRate(engineGetSampleRate());
	bpFilter->setSampleRate(engineGetSampleRate());
	npFilter->setSampleRate(engineGetSampleRate());

	lp2Filter->setSampleRate(engineGetSampleRate());
	hp2Filter->setSampleRate(engineGetSampleRate());
	bp2Filter->setSampleRate(engineGetSampleRate());
	np2Filter->setSampleRate(engineGetSampleRate());

	outLP = lpFilter->processAudioSample(input,1);
	outHP = hpFilter->processAudioSample(input,1);
	outBP = bpFilter->processAudioSample(input,1);
	outNP = npFilter->processAudioSample(input,1);

	out2LP = lp2Filter->processAudioSample(input2, 1);
	out2HP = hp2Filter->processAudioSample(input2, 1);
	out2BP = bp2Filter->processAudioSample(input2, 1);
	out2NP = np2Filter->processAudioSample(input2, 1);


	int sel1 = round(params[FILTERSEL_PARAM].value);
	int sel2 = round(params[FILTER2SEL_PARAM].value);

	for (int i=0;i<4;i++)
	{
	if (sel1 == 0) 
		outputs[OUT1].value = outLP * 5;
	if (sel1 == 1)
		outputs[OUT1].value = outHP * 5;
	if (sel1 == 2)
		outputs[OUT1].value = outBP * 5;
	if (sel1 == 3)
		outputs[OUT1].value = outNP * 5;
	}

	for (int i = 0; i < 4; i++)
	{
		if (sel2 == 0)
			outputs[OUT2].value = out2LP * 5;
		if (sel2 == 1)
			outputs[OUT2].value = out2HP * 5;
		if (sel2 == 2)
			outputs[OUT2].value = out2BP * 5;
		if (sel2 == 3)
			outputs[OUT2].value = out2NP * 5;
	}


	float filter1 = outputs[OUT1].value; 
	float filter2 = outputs[OUT2].value;
	int route = round(params[ROUTE_PARAM].value);
	for (int i = 0; i < 2; i++)
	{
		if (route == 0)
			outputs[MIXOUT].value = (filter1 * ( 1-xfade ))+(filter2 * xfade);
		else
			outputs[MIXOUT].value = 0.0;

}
	
	
}


DualFilterWidget::DualFilterWidget() {
	DualFilter *module = new DualFilter();
	setModule(module);
	box.size = Vec(15 * 18, 380);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/DualFilter.svg")));
		addChild(panel);
	}



int i=140;
int s=27;
int l=10;
int of = -20;
int cv = 310;

	addChild(createScrew<ScrewSilver>(Vec(15, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(createScrew<ScrewSilver>(Vec(15, 365)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 365)));

 
	addChild(createLight<MediumLight<GreenLight>>(Vec(i-45,21),module,DualFilter::FADEA_LIGHTS));
	addChild(createLight<MediumLight<GreenLight>>(Vec(i+23,21),module,DualFilter::FADEB_LIGHTS));

	addParam(createParam<LRoundBlu>(Vec(i -30 , 21), module, DualFilter::FADE_PARAM, 0.0,1.0,0.0));
	addParam(createParam<CKSS>(Vec(i -12, 290), module, DualFilter::ROUTE_PARAM, 0.0, 1.0, 1.0));

	addParam(createParam<LRoundWhy>(Vec(of + 68, 30), module, DualFilter::FREQ_PARAM, minfreq, maxfreq, maxfreq));
	addParam(createParam<LRoundWhy>(Vec(of + 58 + i, 30), module, DualFilter::FREQ2_PARAM, minfreq, maxfreq, maxfreq));

	addParam(createParam<RoundWhy>(Vec(of + 33, 113), module, DualFilter::FREQ_CV_PARAM, -1.0, 1.0, 0.0));
	addParam(createParam<RoundWhy>(Vec(of + 100, 93), module, DualFilter::RES_PARAM, 0.0, 0.99, 0.0));

	addParam(createParam<RoundWhy>(Vec(of + 100 + i, 113), module, DualFilter::FREQ2_CV_PARAM, -1.0, 1.0, 0.0));
	addParam(createParam<RoundWhy>(Vec(of + 33 + i, 93), module, DualFilter::RES2_PARAM, 0.0, 0.99, 0.0));

	addParam(createParam<RoundWhy>(Vec(of + 33, 165), module, DualFilter::FREQ_CV_PARAM2, -1.0, 1.0, 0.0));
	addParam(createParam<RoundWhy>(Vec(of + 100 + i, 165), module, DualFilter::FREQ2_CV_PARAM2, -1.0, 1.0, 0.0));

	addParam(createParam<RoundWhy>(Vec(of + 100, 145), module, DualFilter::DRIVE_PARAM, 0.0, 1.0, 0.0));
	addParam(createParam<RoundWhy>(Vec(of + 33 + i, 145), module, DualFilter::DRIVE2_PARAM, 0.0, 1.0, 0.0));

	addParam(createParam<RoundWhy>(Vec(of + 100, 200), module, DualFilter::VOLA_PARAM, 0.0, 5.0, 0.0));
	addParam(createParam<RoundWhy>(Vec(of + 33 + i, 200), module, DualFilter::VOLB_PARAM, 0.0, 5.0, 0.0));


	addParam(createParam<RoundWhySnapKnob>(Vec(of + 33, 220), module, DualFilter::FILTERSEL_PARAM, 0.0, 3.0, 0.0));
	addParam(createParam<RoundWhySnapKnob>(Vec(of + 100 + i, 220), module, DualFilter::FILTER2SEL_PARAM, 0.0, 3.0, 0.0));




	addInput(createInput<PJ301MCPort>(Vec(l, 276), module, DualFilter::FREQ_INPUT));
	addInput(createInput<PJ301MCPort>(Vec(l + s , 276), module, DualFilter::FREQ_INPUT2));
	addInput(createInput<PJ301MCPort>(Vec(l + s * 2, 276), module, DualFilter::RES_INPUT));
	addInput(createInput<PJ301MCPort>(Vec(l + s * 3, 276), module, DualFilter::DRIVE_INPUT));

	addInput(createInput<PJ301MCPort>(Vec(l + s , cv), module, DualFilter::FADE_CV));
 
	addInput(createInput<PJ301MCPort>(Vec(l + i, 276), module, DualFilter::FREQ2_INPUT));
	addInput(createInput<PJ301MCPort>(Vec(l + s + i, 276), module, DualFilter::FREQ2_INPUT2));
	addInput(createInput<PJ301MCPort>(Vec(l + s * 2 + i, 276), module, DualFilter::RES2_INPUT));
	addInput(createInput<PJ301MCPort>(Vec(l + s * 3 + i, 276), module, DualFilter::DRIVE2_INPUT));

	addInput(createInput<PJ301MOrPort>(Vec(l + s * 2, cv), module, DualFilter::INPUT));
	addInput(createInput<PJ301MOrPort>(Vec(l + s  + i, cv), module, DualFilter::INPUT2));

	addOutput(createOutput<PJ301MOPort>(Vec(l , cv), module, DualFilter::OUT1));

	addOutput(createOutput<PJ301MOPort>(Vec(l + s * 3 + i, cv), module, DualFilter::OUT2));

	addOutput(createOutput<PJ301MOPort>(Vec(l + s * 2 + i, cv), module, DualFilter::MIXOUT));
}

} // namespace rack_plugin_dBiz
