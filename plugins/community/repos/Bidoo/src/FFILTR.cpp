#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"
#include "dsp/ringbuffer.hpp"
#include "dep/filters/vocode.h"

#define BUFF_SIZE 512

using namespace std;

namespace rack_plugin_Bidoo {

struct FFILTR : Module {
	enum ParamIds {
		CUTOFF_PARAM,
		RES_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT,
		CUTOFF_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	DoubleRingBuffer<float,BUFF_SIZE> in_Buffer;
	DoubleRingBuffer<float,BUFF_SIZE> out_Buffer;
	Vocoder *vocoder = NULL;

	FFILTR() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		vocoder = new Vocoder(BUFF_SIZE, 4, engineGetSampleRate());
	}

	~FFILTR() {
		//delete pShifter;
	}

	void step() override;
};


void FFILTR::step() {
	in_Buffer.push(inputs[INPUT].value/10.0f);

	if (in_Buffer.full()) {
		vocoder->process(clamp(params[CUTOFF_PARAM].value + inputs[CUTOFF_INPUT].value*BUFF_SIZE*0.05f ,0.0f, BUFF_SIZE/2.0f ),params[RES_PARAM].value, in_Buffer.startData(), out_Buffer.endData());
		out_Buffer.endIncr(BUFF_SIZE);
		in_Buffer.clear();
	}

	if (out_Buffer.size()>0) {
		outputs[OUTPUT].value = *out_Buffer.startData() * 5.0f;
		out_Buffer.startIncr(1);
	}

}

struct FFILTRWidget : ModuleWidget {
	FFILTRWidget(FFILTR *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/FFILTR.svg")));

		// addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		// addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		addParam(ParamWidget::create<BidooBlueKnob>(Vec(8, 100), module, FFILTR::CUTOFF_PARAM, 0.0f, BUFF_SIZE/2.0f, BUFF_SIZE/2.0f));
		addParam(ParamWidget::create<BidooBlueKnob>(Vec(8, 185), module, FFILTR::RES_PARAM, 1.0f, 10.0f, 1.0f));

		addInput(Port::create<PJ301MPort>(Vec(10, 150.66f), Port::INPUT, module, FFILTR::CUTOFF_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(10, 242.66f), Port::INPUT, module, FFILTR::INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(10, 299), Port::OUTPUT, module, FFILTR::OUTPUT));
	}
};


} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, FFILTR) {
   Model *modelFFILTR = Model::create<FFILTR, FFILTRWidget>("Bidoo", "FFilTr", "FFilTr filter", FILTER_TAG);
   return modelFFILTR;
}
