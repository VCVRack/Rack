#include "Fundamental.hpp"
#include "dsp/digital.hpp"


#define NUM_CHANNELS 10


struct Mutes : Module {
	enum ParamIds {
		MUTE_PARAM,
		NUM_PARAMS = MUTE_PARAM + NUM_CHANNELS
	};
	enum InputIds {
		IN_INPUT,
		NUM_INPUTS = IN_INPUT + NUM_CHANNELS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS = OUT_OUTPUT + NUM_CHANNELS
	};
	enum LightIds {
		MUTE_LIGHT,
		NUM_LIGHTS = MUTE_LIGHT + NUM_CHANNELS
	};

	bool state[NUM_CHANNELS];
	SchmittTrigger muteTrigger[NUM_CHANNELS];

	Mutes() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		onReset();
	}
	void step() override;

	void onReset() override {
		for (int i = 0; i < NUM_CHANNELS; i++) {
			state[i] = true;
		}
	}
	void onRandomize() override {
		for (int i = 0; i < NUM_CHANNELS; i++) {
			state[i] = (randomUniform() < 0.5f);
		}
	}

	json_t *toJson() override {
		json_t *rootJ = json_object();
		// states
		json_t *statesJ = json_array();
		for (int i = 0; i < NUM_CHANNELS; i++) {
			json_t *stateJ = json_boolean(state[i]);
			json_array_append_new(statesJ, stateJ);
		}
		json_object_set_new(rootJ, "states", statesJ);
		return rootJ;
	}
	void fromJson(json_t *rootJ) override {
		// states
		json_t *statesJ = json_object_get(rootJ, "states");
		if (statesJ) {
			for (int i = 0; i < NUM_CHANNELS; i++) {
				json_t *stateJ = json_array_get(statesJ, i);
				if (stateJ)
					state[i] = json_boolean_value(stateJ);
			}
		}
	}
};

void Mutes::step() {
	float out = 0.0f;
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (muteTrigger[i].process(params[MUTE_PARAM + i].value))
			state[i] ^= true;
		if (inputs[IN_INPUT + i].active)
			out = inputs[IN_INPUT + i].value;
		outputs[OUT_OUTPUT + i].value = state[i] ? out : 0.0f;
		lights[MUTE_LIGHT + i].setBrightness(state[i] ? 0.9f : 0.0f);
	}
}


template <typename BASE>
struct MuteLight : BASE {
	MuteLight() {
		this->box.size = mm2px(Vec(6.0f, 6.0f));
	}
};


struct MutesWidget : ModuleWidget {
	MutesWidget(Mutes *module);
};

MutesWidget::MutesWidget(Mutes *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/Mutes.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 18.165)), module, Mutes::MUTE_PARAM + 0, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 28.164)), module, Mutes::MUTE_PARAM + 1, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 38.164)), module, Mutes::MUTE_PARAM + 2, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 48.165)), module, Mutes::MUTE_PARAM + 3, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 58.164)), module, Mutes::MUTE_PARAM + 4, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 68.165)), module, Mutes::MUTE_PARAM + 5, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 78.164)), module, Mutes::MUTE_PARAM + 6, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 88.164)), module, Mutes::MUTE_PARAM + 7, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 98.165)), module, Mutes::MUTE_PARAM + 8, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 108.166)), module, Mutes::MUTE_PARAM + 9, 0.0f, 1.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 17.81)), Port::INPUT, module, Mutes::IN_INPUT + 0));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 27.809)), Port::INPUT, module, Mutes::IN_INPUT + 1));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 37.809)), Port::INPUT, module, Mutes::IN_INPUT + 2));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 47.81)), Port::INPUT, module, Mutes::IN_INPUT + 3));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 57.81)), Port::INPUT, module, Mutes::IN_INPUT + 4));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 67.809)), Port::INPUT, module, Mutes::IN_INPUT + 5));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 77.81)), Port::INPUT, module, Mutes::IN_INPUT + 6));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 87.81)), Port::INPUT, module, Mutes::IN_INPUT + 7));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 97.809)), Port::INPUT, module, Mutes::IN_INPUT + 8));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(4.214, 107.809)), Port::INPUT, module, Mutes::IN_INPUT + 9));

	addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 17.81)), Port::OUTPUT, module, Mutes::OUT_OUTPUT + 0));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 27.809)), Port::OUTPUT, module, Mutes::OUT_OUTPUT + 1));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 37.809)), Port::OUTPUT, module, Mutes::OUT_OUTPUT + 2));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 47.81)), Port::OUTPUT, module, Mutes::OUT_OUTPUT + 3));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 57.809)), Port::OUTPUT, module, Mutes::OUT_OUTPUT + 4));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 67.809)), Port::OUTPUT, module, Mutes::OUT_OUTPUT + 5));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 77.81)), Port::OUTPUT, module, Mutes::OUT_OUTPUT + 6));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 87.81)), Port::OUTPUT, module, Mutes::OUT_OUTPUT + 7));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 97.809)), Port::OUTPUT, module, Mutes::OUT_OUTPUT + 8));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(28.214, 107.809)), Port::OUTPUT, module, Mutes::OUT_OUTPUT + 9));

	addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(mm2px(Vec(17.32, 18.915)), module, Mutes::MUTE_LIGHT + 0));
	addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(mm2px(Vec(17.32, 28.916)), module, Mutes::MUTE_LIGHT + 1));
	addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(mm2px(Vec(17.32, 38.915)), module, Mutes::MUTE_LIGHT + 2));
	addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(mm2px(Vec(17.32, 48.915)), module, Mutes::MUTE_LIGHT + 3));
	addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(mm2px(Vec(17.32, 58.916)), module, Mutes::MUTE_LIGHT + 4));
	addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(mm2px(Vec(17.32, 68.916)), module, Mutes::MUTE_LIGHT + 5));
	addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(mm2px(Vec(17.32, 78.915)), module, Mutes::MUTE_LIGHT + 6));
	addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(mm2px(Vec(17.32, 88.916)), module, Mutes::MUTE_LIGHT + 7));
	addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(mm2px(Vec(17.32, 98.915)), module, Mutes::MUTE_LIGHT + 8));
	addChild(ModuleLightWidget::create<MuteLight<GreenLight>>(mm2px(Vec(17.32, 108.915)), module, Mutes::MUTE_LIGHT + 9));
}


RACK_PLUGIN_MODEL_INIT(Fundamental, Mutes) {
   Model *modelMutes = Model::create<Mutes, MutesWidget>("Fundamental", "Mutes", "Mutes", SWITCH_TAG);
   return modelMutes;
}
