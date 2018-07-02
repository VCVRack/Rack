#include "DS.hpp"

namespace rack_plugin_SubmarineFree {

struct FF_120 : DS_Module {
	static const int deviceCount = 20;
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT,
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
		OUTPUT_13,
		OUTPUT_14,
		OUTPUT_15,
		OUTPUT_16,
		OUTPUT_17,
		OUTPUT_18,
		OUTPUT_19,
		OUTPUT_20,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	int state[deviceCount] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	
	DS_Schmitt schmittTrigger[deviceCount];

	FF_120() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void FF_120::step() {
	if (inputs[INPUT].active) {
		if (schmittTrigger[0].redge(this, inputs[INPUT].value))
			state[0] = !state[0];
	}
	outputs[OUTPUT_1].value = state[0]?voltage1:voltage0;
	for (int i = 1; i < deviceCount; i++) {
		if (schmittTrigger[i].redge(this, state[i-1]?voltage0:voltage1))
					state[i] = !state[i];
		outputs[OUTPUT_1 + i].value = state[i]?voltage1:voltage0;
	}
}

struct FF120 : ModuleWidget {
	FF120(FF_120 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/FF-120.svg")));

		addInput(Port::create<sub_port_blue>(Vec(17.5,19), Port::INPUT, module, FF_120::INPUT));

		for (int i = 0; i < FF_120::deviceCount; i+=2) {
			int offset = 15 * i;

			addOutput(Port::create<sub_port_blue>(Vec(4,53 + offset), Port::OUTPUT, module, FF_120::OUTPUT_1 + i));
			addOutput(Port::create<sub_port_blue>(Vec(31,68 + offset), Port::OUTPUT, module, FF_120::OUTPUT_1 + i + 1));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, FF120) {
   Model *modelFF120 = Model::create<FF_120, FF120>("SubmarineFree", "FF-120", "FF-120 20-Stage Flip-Flop Counter", LOGIC_TAG, MULTIPLE_TAG);
   return modelFF120;
}

