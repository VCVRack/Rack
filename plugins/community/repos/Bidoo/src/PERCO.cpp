// Based on Will Pirkle's courses & Vadim Zavalishin's book

#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/decimator.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

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

struct PERCO : Module {
	enum ParamIds {
		CUTOFF_PARAM,
		Q_PARAM,
		CMOD_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		CUTOFF_INPUT,
		Q_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_LP,
		OUT_BP,
		OUT_HP,
		NUM_OUTPUTS
	};
	enum LightIds {
		LEARN_LIGHT,
		NUM_LIGHTS
	};
	MultiFilter filter;

	PERCO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;

};

void PERCO::step() {
	float cfreq = pow(2.0f,rescale(clamp(params[CUTOFF_PARAM].value + params[CMOD_PARAM].value * inputs[CUTOFF_INPUT].value / 5.0f,0.0f,1.0f),0.0f,1.0f,4.5f,13.0f));
	float q = 10.0f * clamp(params[Q_PARAM].value + inputs[Q_INPUT].value / 5.0f, 0.1f, 1.0f);
	filter.setParams(cfreq,q,engineGetSampleRate());
	float in = inputs[IN].value/5.0f; //normalise to -1/+1 we consider VCV Rack standard is #+5/-5V on VCO1
	//filtering
	filter.calcOutput(in);
	outputs[OUT_LP].value = filter.lp * 5.0f;
	outputs[OUT_HP].value = filter.hp * 5.0f;
	outputs[OUT_BP].value = filter.bp * 5.0f;
}


struct PERCOWidget : ModuleWidget {
	PERCOWidget(PERCO *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/PERCO.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<BidooHugeBlueKnob>(Vec(31, 61), module, PERCO::CUTOFF_PARAM, 0.0f, 1.0f, 1.0f));
		addParam(ParamWidget::create<BidooLargeBlueKnob>(Vec(12, 143), module, PERCO::Q_PARAM, 0.1f, 1.0f, 0.1f));
		addParam(ParamWidget::create<BidooLargeBlueKnob>(Vec(71, 143), module, PERCO::CMOD_PARAM, -1.0f, 1.0f, 0.0f));

		addInput(Port::create<PJ301MPort>(Vec(10, 276), Port::INPUT, module, PERCO::IN));
		addInput(Port::create<PJ301MPort>(Vec(48, 276), Port::INPUT, module, PERCO::CUTOFF_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(85, 276), Port::INPUT, module, PERCO::Q_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(10, 320), Port::OUTPUT, module, PERCO::OUT_LP));
		addOutput(Port::create<PJ301MPort>(Vec(48, 320), Port::OUTPUT, module, PERCO::OUT_BP));
		addOutput(Port::create<PJ301MPort>(Vec(85, 320), Port::OUTPUT, module, PERCO::OUT_HP));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, PERCO) {
   Model *modelPERCO = Model::create<PERCO, PERCOWidget>("Bidoo", "pErCO", "pErCO filter", FILTER_TAG);
   return modelPERCO;
}
