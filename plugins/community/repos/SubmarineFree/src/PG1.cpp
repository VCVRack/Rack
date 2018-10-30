#include "DS.hpp"
#include "dsp/digital.hpp"

namespace rack_plugin_SubmarineFree {

template <int x>
struct PG_1 : DS_Module {
	enum ParamIds {
		PARAM_1,
		NUM_PARAMS = x
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
	DS_Schmitt schmitt[x];
	PulseGenerator pulse[x];

	PG_1() : DS_Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override {
		float deltaTime = 1.0f / engineGetSampleRate();
		for (int i = 0; i < x; i++) {
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
};

struct PG104 : ModuleWidget {
	PG104(PG_1<4> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/PG-104.svg")));

		for (int i = 0; i < 4; i++) {
			int offset = 87 * i;
			addInput(Port::create<BluePort>(Vec(2.5,19 + offset), Port::INPUT, module, PG_1<4>::INPUT_1 + i));

			addOutput(Port::create<BluePort>(Vec(2.5,75 + offset), Port::OUTPUT, module, PG_1<4>::OUTPUT_1 + i));

			addParam(ParamWidget::create<SmallKnob<LightKnob>>(Vec(3,47.5 + offset), module, PG_1<4>::PARAM_1 + i, -5.0f, 2.0f, -2.0f));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

struct PG112 : ModuleWidget {
	PG112(PG_1<12> *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/PG-112.svg")));

		for (int i = 0; i < 12; i++) {
			int offset = 29 * i;
			addInput(Port::create<BluePort>(Vec(4,19 + offset), Port::INPUT, module, PG_1<12>::INPUT_1 + i));

			addOutput(Port::create<BluePort>(Vec(92,19 + offset), Port::OUTPUT, module, PG_1<12>::OUTPUT_1 + i));

			addParam(ParamWidget::create<SmallKnob<LightKnob>>(Vec(33,19.5 + offset), module, PG_1<12>::PARAM_1 + i, -5.0f, 2.0f, -2.0f));
		}
	}
	void appendContextMenu(Menu *menu) override {
		((DS_Module *)module)->appendContextMenu(menu);
	}
};

} // namespace rack_plugin_SubmarineFree

using namespace rack_plugin_SubmarineFree;

RACK_PLUGIN_MODEL_INIT(SubmarineFree, PG104) {
   Model *modelPG104 = Model::create<PG_1<4>, PG104>("Submarine (Free)", "PG-104", "PG-104 Pulse Generators", LOGIC_TAG, MULTIPLE_TAG);
   return modelPG104;
}

RACK_PLUGIN_MODEL_INIT(SubmarineFree, PG112) {
   Model *modelPG112 = Model::create<PG_1<12>, PG112>("Submarine (Free)", "PG-112", "PG-112 Pulse Generators", LOGIC_TAG, MULTIPLE_TAG);
   return modelPG112;
}
