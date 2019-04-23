#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"
#include "dsp/ringbuffer.hpp"
#include "dep/filters/pitchshifter.h"
#include "dsp/fir.hpp"

#define BUFF_SIZE 256
#define OVERLAP 4

using namespace std;

namespace rack_plugin_Bidoo {

struct CURT : Module {
	enum ParamIds {
		PITCH_PARAM,
		MODE_PARAM,
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
	DoubleRingBuffer<float, 2*BUFF_SIZE> out_Buffer;
	float bins[OVERLAP][BUFF_SIZE];
	int index=-1;
	int readSteps=0;
	int writeSteps=0;
	SchmittTrigger modeTrigger;
	bool mode=0;

	CURT() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for(int i=0; i<OVERLAP; i++) {
			memset(bins[i], 0, sizeof(bins[i]));
		}
		for(int i=0; i<BUFF_SIZE; i++) {
			in_Buffer.push(0.0f);
		}
		for(int i=0; i<2*BUFF_SIZE; i++) {
			out_Buffer.push(0.0f);
		}
	}

	~CURT() {

	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "mode", json_boolean(mode));
		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		json_t *modeJ = json_object_get(rootJ, "mode");
		if (modeJ)
			mode = json_is_true(modeJ);
	}

	void step() override;
};

void CURT::step() {
	if (modeTrigger.process(params[MODE_PARAM].value)) {
		mode = !mode;
	}

	in_Buffer.startIncr(1);
	in_Buffer.push(inputs[INPUT].value);

	readSteps++;

	if (readSteps>=(BUFF_SIZE/OVERLAP)) {
		index=(index+1)%OVERLAP;
		for(int i=0; i<BUFF_SIZE; i++) {
			bins[index][i]=*(in_Buffer.startData()+i);
		}
		blackmanHarrisWindow(bins[index],BUFF_SIZE);
		readSteps = 0;
	}

	writeSteps++;

	if ((writeSteps>=((float)BUFF_SIZE*params[PITCH_PARAM].value/(float)OVERLAP))) {
		if ((index%2==0) || (mode)) {
			for(int i=0; i<BUFF_SIZE; i++) {
				out_Buffer.data[out_Buffer.mask(out_Buffer.end-BUFF_SIZE+i)] += bins[index][i];
			}
		}
		else
		{
			for(int i=0; i<BUFF_SIZE; i++) {
				out_Buffer.data[out_Buffer.mask(out_Buffer.end-BUFF_SIZE+i)] += bins[index][BUFF_SIZE-i-1];
			}
		}

		writeSteps = 0;
	}

	outputs[OUTPUT].value = *out_Buffer.startData();
	out_Buffer.startIncr(1);
	out_Buffer.push(0.0f);
}

struct CURTWidget : ModuleWidget {
	CURTWidget(CURT *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/CURT.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		addParam(ParamWidget::create<BidooBlueKnob>(Vec(8, 100), module, CURT::PITCH_PARAM, 0.2f, 2.0f, 1.0f));
		addInput(Port::create<PJ301MPort>(Vec(10, 150.0f), Port::INPUT, module, CURT::PITCH_INPUT));
		addParam(ParamWidget::create<BlueCKD6>(Vec(8, 190.0f), module, CURT::MODE_PARAM, 0.0f, 1.0f, 0.0f));
		addInput(Port::create<PJ301MPort>(Vec(10, 242.66f), Port::INPUT, module, CURT::INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(10, 299), Port::OUTPUT, module, CURT::OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, CURT) {
   Model *modelCURT = Model::create<CURT, CURTWidget>("Bidoo", "cuRt", "cuRt .......", EFFECT_TAG);
   return modelCURT;
}
