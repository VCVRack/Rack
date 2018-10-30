#include <global_pre.hpp>
#include <global.hpp>
#include "DS.hpp"
#include <random>
#include <chrono>

namespace rack_plugin_SubmarineFree {

template <int deviceCount>
struct FF_1 : DS_Module {
	int doResetFlag = 0;
	int doRandomFlag = 0;
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		NUM_OUTPUTS = deviceCount
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	int state[deviceCount] = {};	
	DS_Schmitt schmittTrigger[deviceCount];

	FF_1() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override {
		if (doResetFlag) doReset();
		if (doRandomFlag) doRandomize();
		if (inputs[INPUT].active) {
			if (schmittTrigger[0].redge(this, inputs[INPUT].value))
				state[0] = !state[0];
		}
		outputs[OUTPUT_1].value = state[0]?voltage1:voltage0;
		for (int i = 1; i < deviceCount; i++) {
			if (schmittTrigger[i].fedge(this, state[i-1]?voltage1:voltage0))
						state[i] = !state[i];
			outputs[OUTPUT_1 + i].value = state[i]?voltage1:voltage0;
		}
	}
	void doRandomize() {
		doRandomFlag = 0;
		std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_int_distribution<int> distribution(0,1);
		state[0] = distribution(generator);
		outputs[OUTPUT_1].value = state[0]?voltage1:voltage0;
		for (int i = 1; i < deviceCount; i++) {
			state[i] = distribution(generator);
			schmittTrigger[i].set(state[i-1]);
			outputs[OUTPUT_1 + i].value = state[i]?voltage1:voltage0;
		}
	}
	void doReset() {
		doResetFlag = 0;
		for (int i = 0; i < deviceCount; i++) {
			state[i] = 0;
			if (i) schmittTrigger[i].reset();
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

struct FF110 : ModuleWidget {
	FF110(FF_1<10> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/FF-110.svg")));

		addInput(Port::create<BluePort>(Vec(2.5,19), Port::INPUT, module, FF_1<10>::INPUT));

		for (int i = 0; i < 10; i++) {
			int offset = 29 * i;

			addOutput(Port::create<BluePort>(Vec(2.5,77 + offset), Port::OUTPUT, module, FF_1<10>::OUTPUT_1 + i));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

struct FF120 : ModuleWidget {
	FF120(FF_1<20> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/FF-120.svg")));

		addInput(Port::create<BluePort>(Vec(17.5,19), Port::INPUT, module, FF_1<20>::INPUT));

		for (int i = 0; i < 20; i+=2) {
			int offset = 15 * i;

			addOutput(Port::create<BluePort>(Vec(4,53 + offset), Port::OUTPUT, module, FF_1<20>::OUTPUT_1 + i));
			addOutput(Port::create<BluePort>(Vec(31,68 + offset), Port::OUTPUT, module, FF_1<20>::OUTPUT_1 + i + 1));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, FF110) {
   Model *modelFF110 = Model::create<FF_1<10>, FF110>("Submarine (Free)", "FF-110", "FF-110 10-Stage Flip-Flop Counter", LOGIC_TAG, MULTIPLE_TAG);
   return modelFF110;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, FF120) {
   Model *modelFF120 = Model::create<FF_1<20>, FF120>("Submarine (Free)", "FF-120", "FF-120 20-Stage Flip-Flop Counter", LOGIC_TAG, MULTIPLE_TAG);
   return modelFF120;
}
