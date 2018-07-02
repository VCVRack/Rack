#include "DS.hpp"

namespace rack_plugin_SubmarineFree {

struct FF_212 : DS_Module {
	static const int deviceCount = 12;
	enum ParamIds {
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
	
	int state[deviceCount] = {0,0,0,0,0,0,0,0,0,0,0,0};	
	DS_Schmitt schmittTrigger[deviceCount];

	FF_212() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void FF_212::step() {
	for (int i = 0; i < deviceCount; i++) {
		if (inputs[INPUT_1 + i].active) {
			if (schmittTrigger[i].redge(this, inputs[INPUT_1 + i].value))
				state[i] = !state[i];	
		}
		else {
			if (i) {
				if (schmittTrigger[i].redge(this, state[i-1]?voltage0:voltage1))
					state[i] = !state[i];
			}
		}
		outputs[OUTPUT_1 + i].value = state[i]?voltage1:voltage0;
	}
}

struct FF212 : ModuleWidget {
	FF212(FF_212 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/FF-212.svg")));

		for (int i = 0; i < FF_212::deviceCount; i++) {
			int offset = 29 * i;
			addInput(Port::create<sub_port_blue>(Vec(4,19 + offset), Port::INPUT, module, FF_212::INPUT_1 + i));

			addOutput(Port::create<sub_port_blue>(Vec(62,19 + offset), Port::OUTPUT, module, FF_212::OUTPUT_1 + i));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, FF212) {
   Model *modelFF212 = Model::create<FF_212, FF212>("SubmarineFree", "FF-212", "FF-212 Edge Triggered Flip-Flops", LOGIC_TAG, MULTIPLE_TAG);
   return modelFF212;
}
