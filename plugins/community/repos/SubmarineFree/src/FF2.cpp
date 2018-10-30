#include <global_pre.hpp>
#include <global.hpp>
#include "DS.hpp"
#include <random>
#include <chrono>

namespace rack_plugin_SubmarineFree {

template <int x>
struct FF_2 : DS_Module {
	int doResetFlag = 0;
	int doRandomFlag = 0;
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_1,
		NUM_INPUTS = x
	};
	enum OutputIds {
		OUTPUT_1,
		NUM_OUTPUTS = x
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	int state[x] = {};	
	DS_Schmitt schmittTrigger[x];

	FF_2() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override {
		if (doResetFlag) doReset();
		if (doRandomFlag) doRandomize();
		for (int i = 0; i < x; i++) {
			if (inputs[INPUT_1 + i].active) {
				if (schmittTrigger[i].redge(this, inputs[INPUT_1 + i].value))
					state[i] = !state[i];	
			}
			else {
				if (i) {
					if (schmittTrigger[i].fedge(this, state[i-1]?voltage1:voltage0))
						state[i] = !state[i];
				}
			}
			outputs[OUTPUT_1 + i].value = state[i]?voltage1:voltage0;
		}
	}
	void doRandomize() {
		doRandomFlag = 0;
		std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_int_distribution<int> distribution(0,1);
		for (int i = 0; i < x; i++) {
		 	state[i] = distribution(generator);
			if (i) if (!inputs[INPUT_1 + i].active) schmittTrigger[i].set(state[i-1]);
			outputs[OUTPUT_1 + i].value = state[i]?voltage1:voltage0;
		}
	}
	void doReset() {
		doResetFlag = 0;
		for (int i = 0; i < x; i++) {
			state[i] = 0;
			if (!inputs[INPUT_1 + i].active) schmittTrigger[i].reset();
			outputs[OUTPUT_1 + i].value = voltage0;
		}
	}
	void onRandomize() override {
		if (rack::global->gPaused) {
			doRandomize();
		}
		else {
			doResetFlag = 0;
			doRandomFlag = 1;
		}
	}
	void onReset() override {
		if (rack::global->gPaused) {
			doReset();
		}
		else {
			doRandomFlag = 0;
			doResetFlag = 1;
		}
	}
};

struct FF206 : ModuleWidget {
	FF206(FF_2<6> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/FF-206.svg")));

		for (int i = 0; i < 6; i++) {
			int offset = 58 * i;
			addInput(Port::create<BluePort>(Vec(2.5,19 + offset), Port::INPUT, module, FF_2<6>::INPUT_1 + i));

			addOutput(Port::create<BluePort>(Vec(2.5,47 + offset), Port::OUTPUT, module, FF_2<6>::OUTPUT_1 + i));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

struct FF212 : ModuleWidget {
	FF212(FF_2<12> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/FF-212.svg")));

		for (int i = 0; i < 12; i++) {
			int offset = 29 * i;
			addInput(Port::create<BluePort>(Vec(4,19 + offset), Port::INPUT, module, FF_2<12>::INPUT_1 + i));

			addOutput(Port::create<BluePort>(Vec(62,19 + offset), Port::OUTPUT, module, FF_2<12>::OUTPUT_1 + i));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, FF206) {
   Model *modelFF206 = Model::create<FF_2<6>, FF206>("Submarine (Free)", "FF-206", "FF-206 Edge Triggered Flip-Flops", LOGIC_TAG, MULTIPLE_TAG);
   return modelFF206;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, FF212) {
   Model *modelFF212 = Model::create<FF_2<12>, FF212>("Submarine (Free)", "FF-212", "FF-212 Edge Triggered Flip-Flops", LOGIC_TAG, MULTIPLE_TAG);
   return modelFF212;
}
