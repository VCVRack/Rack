#include "DS.hpp"

namespace rack_plugin_SubmarineFree {

template <int x>
struct XG_1 : DS_Module {
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

	XG_1() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override {
		int setCount = 0;
		for (int i = 0; i < x; i++) {
			if (inputs[INPUT_A_1 + i].active)
				if (inputs[INPUT_A_1 + i].value > midpoint())
					setCount++;
			if (inputs[INPUT_B_1 + i].active)
				if (inputs[INPUT_B_1 + i].value > midpoint())
					setCount++;
			if (outputs[OUTPUT_1 + i].active) {
				outputs[OUTPUT_1 + i].value = (setCount % 2)?voltage1:voltage0;
				setCount = 0;
			}
		}
	}
};

struct XG104 : ModuleWidget {
	XG104(XG_1<4> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/XG-104.svg")));

		for (int i = 0; i < 4; i++) {
			int offset = 87 * i;
			addInput(Port::create<BluePort>(Vec(2.5,19 + offset), Port::INPUT, module, XG_1<4>::INPUT_A_1 + i));
			addInput(Port::create<BluePort>(Vec(2.5,47 + offset), Port::INPUT, module, XG_1<4>::INPUT_B_1 + i));

			addOutput(Port::create<BluePort>(Vec(2.5,75 + offset), Port::OUTPUT, module, XG_1<4>::OUTPUT_1 + i));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

struct XG106 : ModuleWidget {
	XG106(XG_1<6> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/XG-106.svg")));

		for (int i = 0; i < 6; i++) {
			int offset = 58 * i;
			addInput(Port::create<BluePort>(Vec(4,19 + offset), Port::INPUT, module, XG_1<6>::INPUT_A_1 + i));
			addInput(Port::create<BluePort>(Vec(4,47 + offset), Port::INPUT, module, XG_1<6>::INPUT_B_1 + i));

			addOutput(Port::create<BluePort>(Vec(62,33 + offset), Port::OUTPUT, module, XG_1<6>::OUTPUT_1 + i));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, XG104) {
   Model *modelXG104 = Model::create<XG_1<4>, XG104>("Submarine (Free)", "XG-104", "XG-104 XOR Gates", LOGIC_TAG, MULTIPLE_TAG);
   return modelXG104;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, XG106) {
   Model *modelXG106 = Model::create<XG_1<6>, XG106>("Submarine (Free)", "XG-106", "XG-106 XOR Gates", LOGIC_TAG, MULTIPLE_TAG);
   return modelXG106;
}
