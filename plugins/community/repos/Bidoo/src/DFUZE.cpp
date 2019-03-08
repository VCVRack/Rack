#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dep/gverb/include/gverb.h"
#include "dep/gverb/src/gverb.c"
#include "dep/gverb/src/gverbdsp.c"
#include "dsp/samplerate.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct DFUZE : Module {
	enum ParamIds {
		SIZE_PARAM,
		REVTIME_PARAM,
		DAMP_PARAM,
		FREEZE_PARAM,
		BANDWIDTH_PARAM,
		EARLYLEVEL_PARAM,
		TAIL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		SIZE_INPUT,
		REVTIME_INPUT,
		DAMP_INPUT,
		FREEZE_INPUT,
		BANDWIDTH_INPUT,
		EARLYLEVEL_INPUT,
		TAIL_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_L_OUTPUT,
		OUT_R_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


	ty_gverb *verb;

	float sr = engineGetSampleRate();

	DFUZE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		verb = gverb_new(engineGetSampleRate(), 1, 1, 1, 1, 1, 1, 1, 1);
	}

	~DFUZE() {
		gverb_free(verb);
	}

	void step() override;
};

void DFUZE::step() {
	gverb_set_roomsize(verb, clamp(params[SIZE_PARAM].value+inputs[SIZE_INPUT].value,0.0f,6.0f));
	gverb_set_revtime(verb, clamp(params[REVTIME_PARAM].value+inputs[REVTIME_INPUT].value,0.0f,10.0f));
	gverb_set_damping(verb, clamp(params[DAMP_PARAM].value+inputs[DAMP_INPUT].value,0.0f,0.9f));
	gverb_set_inputbandwidth(verb, clamp(params[BANDWIDTH_PARAM].value+inputs[BANDWIDTH_INPUT].value,0.0f,1.0f));
	gverb_set_earlylevel(verb, clamp(params[EARLYLEVEL_PARAM].value+inputs[EARLYLEVEL_INPUT].value,0.0f,10.0f));
	gverb_set_taillevel(verb, clamp(params[TAIL_PARAM].value+inputs[TAIL_INPUT].value,0.0f,10.0f));

	gverb_do(verb, inputs[IN_INPUT].value/10.0f, &outputs[OUT_L_OUTPUT].value, &outputs[OUT_R_OUTPUT].value);
}

struct DFUZEWidget : ModuleWidget {
	DFUZEWidget(DFUZE *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/DFUZE.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 50), module, DFUZE::SIZE_PARAM, 0.0f, 6.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 95), module, DFUZE::REVTIME_PARAM, 0.0f, 10.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 140), module, DFUZE::DAMP_PARAM, 0.0f, 0.9f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 185), module, DFUZE::BANDWIDTH_PARAM, 0.0f, 1.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 230), module, DFUZE::EARLYLEVEL_PARAM, 0.0f, 10.0f, 0.5f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(13, 275), module, DFUZE::TAIL_PARAM, 0.0f, 10.0f, 0.5f));


		addInput(Port::create<PJ301MPort>(Vec(65.0f, 52.0f), Port::INPUT, module, DFUZE::SIZE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 97.0f), Port::INPUT, module, DFUZE::REVTIME_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 142.0f), Port::INPUT, module, DFUZE::DAMP_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 187.0f), Port::INPUT, module, DFUZE::BANDWIDTH_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 232.0f), Port::INPUT, module, DFUZE::EARLYLEVEL_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(65.0f, 277.0f), Port::INPUT, module, DFUZE::TAIL_INPUT));

	 	//Changed ports opposite way around
		addInput(Port::create<TinyPJ301MPort>(Vec(24.0f, 329.0f), Port::INPUT, module, DFUZE::IN_INPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 319.0f),Port::OUTPUT, module, DFUZE::OUT_L_OUTPUT));
		addOutput(Port::create<TinyPJ301MPort>(Vec(78.0f, 339.0f),Port::OUTPUT, module, DFUZE::OUT_R_OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, DFUZE) {
   Model *modelDFUZE = Model::create<DFUZE, DFUZEWidget>("Bidoo", "dFUZE", "dFUZE reverberator", REVERB_TAG, EFFECT_TAG);
   return modelDFUZE;
}
