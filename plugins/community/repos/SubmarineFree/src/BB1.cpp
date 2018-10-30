#include <global_pre.hpp>
#include <global.hpp>
#include "DS.hpp"
#include <random>
#include <chrono>

namespace rack_plugin_SubmarineFree {

template <int x>
struct BB_1 : DS_Module {
	int doResetFlag = 0;
	int doRandomFlag = 0;
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
		NUM_OUTPUTS = x
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	float sample[x] = {};	
	DS_Schmitt schmittTrigger;

	BB_1() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override {
		if (doResetFlag) doReset();
		if (doRandomFlag) doRandomize();
		int triggered = true;
		if (inputs[INPUT_CLK].active) {
			triggered = schmittTrigger.redge(this, inputs[INPUT_CLK].value);
		}
		if (triggered) {
			for (int i = x - 1; i; i--)
				sample[i] = sample[i - 1];
			sample[0] = inputs[INPUT_CV].value;
		}
		for (int i = 0; i < x; i++)
			outputs[OUTPUT_1 + i].value = sample[i];
	}
	void doRandomize() {
		doRandomFlag = 0;
		std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
		std::uniform_real_distribution<float> distribution(voltage0, voltage1);	
		for (int i = 0; i < x; i++) {
			outputs[OUTPUT_1 + i].value = sample[i] = distribution(generator); 
		}
	}
	void doReset() {
		doResetFlag = 0;
		for (int i = 0; i < x; i++)
			outputs[OUTPUT_1 + i].value = sample[i] = 0.0f;
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

struct BB120 : ModuleWidget {
	BB120(BB_1<20> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/BB-120.svg")));

		addInput(Port::create<BluePort>(Vec(4.5,19), Port::INPUT, module, BB_1<20>::INPUT_CLK));
		addInput(Port::create<SilverPort>(Vec(31.5,34), Port::INPUT, module, BB_1<20>::INPUT_CV));

		for (int i = 0; i < 20; i+=2) {
			int offset = 15 * i;

			addOutput(Port::create<SilverPort>(Vec(4,53 + offset), Port::OUTPUT, module, BB_1<20>::OUTPUT_1 + i));
			addOutput(Port::create<SilverPort>(Vec(31,68 + offset), Port::OUTPUT, module, BB_1<20>::OUTPUT_1 + i + 1));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, BB120) {
   Model *modelBB120 = Model::create<BB_1<20>, BB120>("Submarine (Free)", "BB-120", "BB-120 20-Stage Bucket Brigade Sample and Hold", LOGIC_TAG, DELAY_TAG, SAMPLE_AND_HOLD_TAG, MULTIPLE_TAG);
   return modelBB120;
}
