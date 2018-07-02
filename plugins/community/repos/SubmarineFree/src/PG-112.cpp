#include "DS.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_SubmarineFree {

struct PG_112 : DS_Module {
	static const int deviceCount = 12;
	enum ParamIds {
		PARAM_1,
		PARAM_2,
		PARAM_3,
		PARAM_4,
		PARAM_5,
		PARAM_6,
		PARAM_7,
		PARAM_8,
		PARAM_9,
		PARAM_10,
		PARAM_11,
		PARAM_12,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_1,
		INPUT_2,
		INPUT_3,
		INPUT_4,
		INPUT_5,
		INPUT_6,
		INPUT_7,
		INPUT_8,
		INPUT_9,
		INPUT_10,
		INPUT_11,
		INPUT_12,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		OUTPUT_5,
		OUTPUT_6,
		OUTPUT_7,
		OUTPUT_8,
		OUTPUT_9,
		OUTPUT_10,
		OUTPUT_11,
		OUTPUT_12,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	DS_Schmitt schmitt[deviceCount];
	PulseGenerator pulse[deviceCount];

	PG_112() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void PG_112::step() {
	float deltaTime = 1.0f / engineGetSampleRate();
	for (int i = 0; i < deviceCount; i++) {
		if (schmitt[i].redge(this, inputs[INPUT_1 + i].value)) {
			pulse[i].process(deltaTime);
			pulse[i].trigger(powf(10.0f, params[PARAM_1 + i].value));
			outputs[OUTPUT_1 + i].value = voltage1;
		}
		else {
			outputs[OUTPUT_1 + i].value = pulse[i].process(deltaTime)?voltage1:voltage0;
		}
	}
}

struct PG112 : ModuleWidget {
	PG112(PG_112 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/PG-112.svg")));

		for (int i = 0; i < PG_112::deviceCount; i++) {
			int offset = 29 * i;
			addInput(Port::create<sub_port_blue>(Vec(4,19 + offset), Port::INPUT, module, PG_112::INPUT_1 + i));

			addOutput(Port::create<sub_port_blue>(Vec(92,19 + offset), Port::OUTPUT, module, PG_112::OUTPUT_1 + i));

			addParam(ParamWidget::create<sub_knob_small>(Vec(33,19.5 + offset), module, PG_112::PARAM_1 + i, -5.0f, 2.0f, -2.0f));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, PG112) {
   Model *modelPG112 = Model::create<PG_112, PG112>("SubmarineFree", "PG-112", "PG-112 Pulse Generators", LOGIC_TAG, MULTIPLE_TAG);
   return modelPG112;
}
