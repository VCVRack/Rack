#include "DS.hpp"

namespace rack_plugin_SubmarineFree {

struct FF_110 : DS_Module {
	static const int deviceCount = 10;
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
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	int state[deviceCount] = {0,0,0,0,0,0,0,0,0,0};	
	DS_Schmitt schmittTrigger[deviceCount];

	FF_110() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void FF_110::step() {
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

struct FF110 : ModuleWidget {
	FF110(FF_110 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/FF-110.svg")));

		addInput(Port::create<sub_port_blue>(Vec(2.5,19), Port::INPUT, module, FF_110::INPUT));

		for (int i = 0; i < FF_110::deviceCount; i++) {
			int offset = 29 * i;

			addOutput(Port::create<sub_port_blue>(Vec(2.5,77 + offset), Port::OUTPUT, module, FF_110::OUTPUT_1 + i));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, FF110) {
   Model *modelFF110 = Model::create<FF_110, FF110>("SubmarineFree", "FF-110", "FF-110 10-Stage Flip-Flop Counter", LOGIC_TAG, MULTIPLE_TAG);
   return modelFF110;
}
