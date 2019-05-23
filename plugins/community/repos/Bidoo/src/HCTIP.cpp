#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"
#include "dsp/ringbuffer.hpp"
#include "dep/filters/pitchshifter.h"

#define BUFF_SIZE 2048

using namespace std;

namespace rack_plugin_Bidoo {

struct HCTIP : Module {
	enum ParamIds {
		PITCH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT,
		PITCH_INPUT,
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
	PitchShifter *pShifter = NULL;

	HCTIP() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		pShifter = new PitchShifter(BUFF_SIZE, 8, engineGetSampleRate());
	}

	~HCTIP() {
		delete pShifter;
	}

	void step() override;
};


void HCTIP::step() {
	in_Buffer.push(inputs[INPUT].active ? inputs[INPUT].value/10.0f : 0.0f);

	if (in_Buffer.full()) {
		pShifter->process(clamp(params[PITCH_PARAM].value + inputs[PITCH_INPUT].value ,0.5f,2.0f), in_Buffer.startData(), out_Buffer.endData());
		out_Buffer.endIncr(BUFF_SIZE);
		in_Buffer.clear();
	}

	if (out_Buffer.size()>0) {
		outputs[OUTPUT].value = *out_Buffer.startData() * 5.0f * (inputs[INPUT].active ? 1.0f : 0.0f);
		out_Buffer.startIncr(1);
	}

}

struct HCTIPWidget : ModuleWidget {
	HCTIPWidget(HCTIP *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/HCTIP.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		addParam(ParamWidget::create<BidooBlueKnob>(Vec(8, 100), module, HCTIP::PITCH_PARAM, 0.5f, 2.0f, 1.0f));

		addInput(Port::create<PJ301MPort>(Vec(10, 150.66f), Port::INPUT, module, HCTIP::PITCH_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(10, 242.66f), Port::INPUT, module, HCTIP::INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(10, 299), Port::OUTPUT, module, HCTIP::OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, HCTIP) {
   Model *modelHCTIP = Model::create<HCTIP, HCTIPWidget>("Bidoo", "HCTIP", "HCTIP pitch shifter", EFFECT_TAG);
   return modelHCTIP;
}
