#include "Bidoo.hpp"
#include "BidooComponents.hpp"

using namespace std;

namespace rack_plugin_Bidoo {

struct MS : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		L_INPUT,
		R_INPUT,
		M_INPUT,
		S_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		L_OUTPUT,
		R_OUTPUT,
		M_OUTPUT,
		S_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	MS() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {	}

	void step() override;
};


void MS::step() {
	outputs[S_OUTPUT].value = 0.5f * (inputs[L_INPUT].value - inputs[R_INPUT].value);
	outputs[M_OUTPUT].value = 0.5f * (inputs[L_INPUT].value + inputs[R_INPUT].value);
	outputs[L_OUTPUT].value = inputs[M_INPUT].value + inputs[S_INPUT].value;
	outputs[R_OUTPUT].value = inputs[M_INPUT].value - inputs[S_INPUT].value;
}

struct MSWidget : ModuleWidget {
	MSWidget(MS *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/MS.svg")));

		addInput(Port::create<PJ301MPort>(Vec(10, 30), Port::INPUT, module, MS::L_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(10, 70), Port::INPUT, module, MS::R_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(10, 110), Port::OUTPUT, module, MS::M_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(10, 150), Port::OUTPUT, module, MS::S_OUTPUT));
		addInput(Port::create<PJ301MPort>(Vec(10, 190), Port::INPUT, module, MS::M_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(10, 230), Port::INPUT, module, MS::S_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(10, 270), Port::OUTPUT, module, MS::L_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(10, 310), Port::OUTPUT, module, MS::R_OUTPUT));
	}
};

} // namespace rack_plugin_Bidoo

using namespace rack_plugin_Bidoo;

RACK_PLUGIN_MODEL_INIT(Bidoo, MS) {
   Model *modelMS = Model::create<MS, MSWidget>("Bidoo", "MS", "MS Mid/Side decoder/encoder", PANNING_TAG, MIXER_TAG);
   return modelMS;
}
