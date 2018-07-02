#include "DS.hpp"

namespace rack_plugin_SubmarineFree {

struct BB_120 : DS_Module {
	static const int deviceCount = 20;
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_CLK,
		INPUT_CV,
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
	
	float sample[deviceCount] = {};	
	DS_Schmitt schmittTrigger;

	BB_120() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};

void BB_120::step() {
	int triggered = true;
	if (inputs[INPUT_CLK].active) {
		triggered = schmittTrigger.redge(this, inputs[INPUT_CLK].value);
	}
	if (triggered) {
		for (int i = deviceCount - 1; i; i--)
			sample[i] = sample[i - 1];
		sample[0] = inputs[INPUT_CV].value;
	}
	for (int i = 0; i < deviceCount; i++)
		outputs[OUTPUT_1 + i].value = sample[i];
}

struct BB120 : ModuleWidget {
	BB120(BB_120 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BB-120.svg")));

		addInput(Port::create<sub_port_blue>(Vec(4.5,19), Port::INPUT, module, BB_120::INPUT_CLK));
		addInput(Port::create<sub_port>(Vec(31.5,34), Port::INPUT, module, BB_120::INPUT_CV));

		for (int i = 0; i < BB_120::deviceCount; i+=2) {
			int offset = 15 * i;

			addOutput(Port::create<sub_port>(Vec(4,53 + offset), Port::OUTPUT, module, BB_120::OUTPUT_1 + i));
			addOutput(Port::create<sub_port>(Vec(31,68 + offset), Port::OUTPUT, module, BB_120::OUTPUT_1 + i + 1));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BB120) {
   Model *modelBB120 = Model::create<BB_120, BB120>("SubmarineFree", "BB-120", "BB-120 20-Stage Bucket Brigade Sample and Hold", LOGIC_TAG, DELAY_TAG, SAMPLE_AND_HOLD_TAG, MULTIPLE_TAG);
   return modelBB120;
}
