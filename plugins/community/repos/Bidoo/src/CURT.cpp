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
		BUFF_SIZE_PARAM,
		OVERLAP_PARAM,
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
	size_t readSteps=0;
	size_t writeSteps=0;
	SchmittTrigger modeTrigger;
	bool mode=0;
	size_t overlap, buff_size;

	CURT() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		overlap = OVERLAP;
		buff_size = BUFF_SIZE;
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

	void updateBuff() {
		while (in_Buffer.size()<buff_size) {
			in_Buffer.push(0.0f);
		}
		while (in_Buffer.size()>buff_size) {
			in_Buffer.startIncr(1);
		}
		while (out_Buffer.size()<2*buff_size) {
			in_Buffer.push(0.0f);
		}
		while (out_Buffer.size()>2*buff_size) {
			in_Buffer.startIncr(1);
		}
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

	if ((size_t)params[BUFF_SIZE_PARAM].value != buff_size) {
		buff_size = pow(2.0f, params[BUFF_SIZE_PARAM].value);
		updateBuff();
	}

	if ((size_t)params[OVERLAP_PARAM].value != overlap) {
		overlap = params[OVERLAP_PARAM].value;
	}

	in_Buffer.startIncr(1);
	in_Buffer.push(inputs[INPUT].value);

	readSteps++;

	if (readSteps>=(buff_size/overlap)) {
		index=(index+1)%overlap;
		for(size_t i=0; i<buff_size; i++) {
			bins[index][i]=*(in_Buffer.startData()+i);
		}
		blackmanHarrisWindow(bins[index],buff_size);
		readSteps = 0;
	}

	writeSteps++;

	if ((writeSteps>=((float)buff_size*params[PITCH_PARAM].value/(float)overlap))) {
		if ((index%2==0) || (mode)) {
			for(size_t i=0; i<buff_size; i++) {
				out_Buffer.data[out_Buffer.mask(out_Buffer.end-buff_size+i)] += bins[index][i];
			}
		}
		else
		{
			for(size_t i=0; i<buff_size; i++) {
				out_Buffer.data[out_Buffer.mask(out_Buffer.end-buff_size+i)] += bins[index][buff_size-i-1];
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


		addParam(ParamWidget::create<BidooBlueKnob>(Vec(8, 90), module, CURT::PITCH_PARAM, 0.2f, 2.0f, 1.0f));
		addInput(Port::create<PJ301MPort>(Vec(10, 140.0f), Port::INPUT, module, CURT::PITCH_INPUT));
		addParam(ParamWidget::create<BlueCKD6>(Vec(8, 175.0f), module, CURT::MODE_PARAM, 0.0f, 1.0f, 0.0f));
		addParam(ParamWidget::create<BidooBlueSnapTrimpot>(Vec(2, 205), module, CURT::BUFF_SIZE_PARAM, 6.0f, 8.0f, 8.0f));
		addParam(ParamWidget::create<BidooBlueSnapTrimpot>(Vec(25, 205), module, CURT::OVERLAP_PARAM, 1.0f, 4.0f, 2.0f));

		addInput(Port::create<PJ301MPort>(Vec(10, 245.66f), Port::INPUT, module, CURT::INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(10, 299), Port::OUTPUT, module, CURT::OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, CURT) {
   Model *modelCURT = Model::create<CURT, CURTWidget>("Bidoo", "cuRt", "cuRt .......", EFFECT_TAG);
   return modelCURT;
}
