#include "Bidoo.hpp"
#include "BidooComponents.hpp"
#include "dsp/digital.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct SIGMA : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		IN_INPUT,
		NUM_INPUTS = IN_INPUT + 18
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS = OUT_OUTPUT + 6
	};
	enum LightIds {
		NUM_LIGHTS
	};

	SIGMA() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
	}

	void step() override;
};

void SIGMA::step() {
	for (int i = 0; i < NUM_OUTPUTS; i++) {
		outputs[i].value = inputs[3*i].value + inputs[3*i+1].value + inputs[3*i+2].value;
	}
}

struct SIGMAWidget : ModuleWidget {
	SIGMAWidget(SIGMA *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SIGMA.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for (int i = 0; i < SIGMA::NUM_OUTPUTS; i++) {
			addOutput(Port::create<TinyPJ301MPort>(Vec(15.0f+round(i/3)*30.0f, 120.0f+(i%3)*100.0f),Port::OUTPUT, module, i));
		}

		for (int i = 0; i < SIGMA::NUM_INPUTS; i++) {
			addInput(Port::create<TinyPJ301MPort>(Vec(15.0f+round(i/9)*30.0f, 50.0f+(i%9)*20.0f+round((i%9)/3)*40.0f),Port::INPUT, module, i));
		}

  }
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, SIGMA) {
   Model *modelSIGMA= Model::create<SIGMA, SIGMAWidget>("Bidoo", "Σ", "Σ multiprise :)", MULTIPLE_TAG);
   return modelSIGMA;
}
