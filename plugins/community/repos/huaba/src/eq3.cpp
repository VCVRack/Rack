#include "huaba.hpp"
#include "eq3.hpp"
#include "math.h"

namespace rack_plugin_huaba {

struct EQ3 : Module {
	enum ParamIds {
		LOW_PARAM,
		MID_PARAM,
		HIGH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		CV1_INPUT,
		CV2_INPUT,
		CV3_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	EQSTATE *eq=new EQSTATE();
	const double vsa = (1.0 / 4294967295.0);   // Denormal Fix

	EQ3() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	
		init_3band_state(eq, 880, 5000, engineGetSampleRate());
	}
	void step() override;

	void init_3band_state(EQSTATE* es, int lowfreq, int highfreq, int mixfreq) {
		// Clear state
		memset(es,0,sizeof(EQSTATE));

		// Set Low/Mid/High gains to unity
		es->lg = 1.0;
		es->mg = 1.0;
		es->hg = 1.0;

		// Calculate filter cutoff frequencies
		es->lf = 2 * sin(M_PI * ((double)lowfreq / (double)mixfreq));
		es->hf = 2 * sin(M_PI * ((double)highfreq / (double)mixfreq));
	}

	double do_3band(EQSTATE* es, double sample) {
		// Low / Mid / High - Sample Values
		double  l,m,h;      

		// Filter #1 (lowpass)
		es->f1p0 += (es->lf * (sample   - es->f1p0)) + vsa;
		es->f1p1 += (es->lf * (es->f1p0 - es->f1p1));
		es->f1p2 += (es->lf * (es->f1p1 - es->f1p2));
		es->f1p3 += (es->lf * (es->f1p2 - es->f1p3));
		l = es->f1p3;

		// Filter #2 (highpass)
		es->f2p0 += (es->hf * (sample   - es->f2p0)) + vsa;
		es->f2p1 += (es->hf * (es->f2p0 - es->f2p1));
		es->f2p2 += (es->hf * (es->f2p1 - es->f2p2));
		es->f2p3 += (es->hf * (es->f2p2 - es->f2p3));
		h = es->sdm3 - es->f2p3;

		// Calculate midrange (signal - (low + high))
		m = es->sdm3 - (h + l);

		// Scale, Combine and store
		l *= es->lg;
		m *= es->mg;
		h *= es->hg;

		// Shuffle history buffer
		es->sdm3   = es->sdm2;
		es->sdm2   = es->sdm1;
		es->sdm1   = sample;                

		// Return result
		return(l + m + h);
	}
};

void EQ3::step() {
	
	eq->lg = clamp(params[LOW_PARAM].value + inputs[CV3_INPUT].value / 10.0f, 0.0f, 2.0f);
	eq->mg = clamp(params[MID_PARAM].value + inputs[CV2_INPUT].value / 10.0f, 0.0f, 2.0f);
	eq->hg = clamp(params[HIGH_PARAM].value + inputs[CV1_INPUT].value / 10.0f, 0.0f, 2.0f);

	if (outputs[OUT1_OUTPUT].active && inputs[IN1_INPUT].active)
		outputs[OUT1_OUTPUT].value = do_3band(eq, inputs[IN1_INPUT].value);
	if (outputs[OUT2_OUTPUT].active && inputs[IN2_INPUT].active)
		outputs[OUT2_OUTPUT].value = do_3band(eq, inputs[IN2_INPUT].value);
}

struct EQ3Widget : ModuleWidget {
	EQ3Widget(EQ3 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/EQ3.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<RoundBlackKnob>(Vec(8, 56), module, EQ3::HIGH_PARAM, 0.0, 2.0, 1.0));
		addInput(Port::create<PJ301MPort>(Vec(10.5, 89), Port::INPUT, module, EQ3::CV1_INPUT));

		addParam(ParamWidget::create<RoundBlackKnob>(Vec(8, 136), module, EQ3::MID_PARAM, 0.0, 2.0, 1.0));
		addInput(Port::create<PJ301MPort>(Vec(10.5, 169), Port::INPUT, module, EQ3::CV2_INPUT));

		addParam(ParamWidget::create<RoundBlackKnob>(Vec(8, 215), module, EQ3::LOW_PARAM, 0.0, 2.0, 1.0));
		addInput(Port::create<PJ301MPort>(Vec(10.5, 248), Port::INPUT, module, EQ3::CV3_INPUT));
		
		addInput(Port::create<PJ301MPort>(Vec(10.5, 280), Port::INPUT, module, EQ3::IN1_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(10.5, 320), Port::OUTPUT, module, EQ3::OUT1_OUTPUT));
	}
};

} // namespace rack_plugin_huaba

using namespace rack_plugin_huaba;

RACK_PLUGIN_MODEL_INIT(huaba, EQ3) {
   Model *modelEQ3 = Model::create<EQ3, EQ3Widget>("huaba", "EQ3", "EQ3", EQUALIZER_TAG);
   return modelEQ3;
}
