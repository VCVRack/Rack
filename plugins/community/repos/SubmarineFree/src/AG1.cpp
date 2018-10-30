#include "DS.hpp"

namespace rack_plugin_SubmarineFree {

template <int x>
struct AG_1 : DS_Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_A_1,
		INPUT_B_1 = x,
		NUM_INPUTS = x + x
	};
	enum OutputIds {
		OUTPUT_1,
		NUM_OUTPUTS = x
	};
	enum LightIds {
		NUM_LIGHTS
	};

	AG_1() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override {
		int connCount = 0;
		int setCount = 0;
		for (int i = 0; i < x; i++) {
			if (inputs[INPUT_A_1 + i].active) {
				connCount++;
				if (inputs[INPUT_A_1 + i].value > midpoint())
					setCount++;
			}
			if (inputs[INPUT_B_1 + i].active) {
				connCount++;
				if (inputs[INPUT_B_1 + i].value > midpoint())
					setCount++;
			}
			if (outputs[OUTPUT_1 + i].active) {
				if (connCount)
					outputs[OUTPUT_1 + i].value = (connCount == setCount)?voltage1:voltage0;
				else
					outputs[OUTPUT_1 + i].value = voltage0;
				connCount = 0;
				setCount = 0;
			}
		}
	}
};

struct AG104 : ModuleWidget {
	AG104(AG_1<4> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/AG-104.svg")));

		for (int i = 0; i < 4; i++) {
			int offset = 87 * i;
			addInput(Port::create<BluePort>(Vec(2.5,19 + offset), Port::INPUT, module, AG_1<4>::INPUT_A_1 + i));
			addInput(Port::create<BluePort>(Vec(2.5,47 + offset), Port::INPUT, module, AG_1<4>::INPUT_B_1 + i));

			addOutput(Port::create<BluePort>(Vec(2.5,75 + offset), Port::OUTPUT, module, AG_1<4>::OUTPUT_1 + i));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

struct AG106 : ModuleWidget {
	AG106(AG_1<6> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/AG-106.svg")));

		for (int i = 0; i < 6; i++) {
			int offset = 58 * i;
			addInput(Port::create<BluePort>(Vec(4,19 + offset), Port::INPUT, module, AG_1<6>::INPUT_A_1 + i));
			addInput(Port::create<BluePort>(Vec(4,47 + offset), Port::INPUT, module, AG_1<6>::INPUT_B_1 + i));

			addOutput(Port::create<BluePort>(Vec(62,33 + offset), Port::OUTPUT, module, AG_1<6>::OUTPUT_1 + i));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, AG104) {
   Model *modelAG104 = Model::create<AG_1<4>, AG104>("Submarine (Free)", "AG-104", "AG-104 AND Gates", LOGIC_TAG, MULTIPLE_TAG);
   return modelAG104;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, AG106) {
   Model *modelAG106 = Model::create<AG_1<6>, AG106>("Submarine (Free)", "AG-106", "AG-106 AND Gates", LOGIC_TAG, MULTIPLE_TAG);
   return modelAG106;
}
