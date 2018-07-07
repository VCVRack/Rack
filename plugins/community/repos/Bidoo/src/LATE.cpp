#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"
#include <ctime>

using namespace std;

namespace rack_plugin_Bidoo {

struct LATE : Module {
	enum ParamIds {
		SWING_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SWING_INPUT,
		CLOCK_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CLOCK_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	bool odd = true;
	bool armed = false;
	SchmittTrigger clockTrigger;
	SchmittTrigger resetTrigger;
	clock_t tCurrent = clock();
	clock_t tPrevious = clock();


	LATE() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	}

	void step() override;
};


void LATE::step() {
	outputs[CLOCK_OUTPUT].value = 0;
	clock_t now = clock();

	if (resetTrigger.process(inputs[RESET_INPUT].value)) {
		odd=true;
	}

	if (clockTrigger.process(inputs[CLOCK_INPUT].value)) {
		tPrevious = tCurrent;
		tCurrent = now;
		if (odd) {
			outputs[CLOCK_OUTPUT].value = 10.0f;
			odd = false;
			armed = false;
		}
		else {
			armed = true;
		}
	}

	float lag = rescale(clamp(params[SWING_PARAM].value + inputs[SWING_INPUT].value,0.0f,9.0f),0.0f,10.0f,0.0f,(float)tCurrent-(float)tPrevious);

	if (armed && !odd && (((float)now - (float)tCurrent) >= lag)) {
		outputs[CLOCK_OUTPUT].value = 10.0f;
		armed = false;
		odd = true;
	}
}

struct LATEWidget : ModuleWidget {
	LATEWidget(LATE *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/LATE.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		addParam(ParamWidget::create<BidooBlueKnob>(Vec(8, 70), module, LATE::SWING_PARAM, 0, 9, 0));
		addInput(Port::create<PJ301MPort>(Vec(10, 110), Port::INPUT, module, LATE::SWING_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(10, 173), Port::INPUT, module, LATE::RESET_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(10, 235), Port::INPUT, module, LATE::CLOCK_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(10, 299), Port::OUTPUT, module, LATE::CLOCK_OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, LATE) {
   Model *modelLATE = Model::create<LATE, LATEWidget>("Bidoo", "lATe", "lATe clock modulator", CLOCK_MODULATOR_TAG);
   return modelLATE;
}
