#include "Fundamental.hpp"
#include "dsp/digital.hpp"
#include "dsp/filter.hpp"


template <int TYPE>
struct SequentialSwitch : Module {
	enum ParamIds {
		CHANNELS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RESET_INPUT,
		ENUMS(IN_INPUT, TYPE == 1 ? 1 : 4),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT_OUTPUT, TYPE == 1 ? 4 : 1),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(CHANNEL_LIGHT, 4),
		NUM_LIGHTS
	};

	SchmittTrigger clockTrigger;
	SchmittTrigger resetTrigger;
	int channel = 0;
	SlewLimiter channelFilter[4];

	SequentialSwitch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
		for (int i = 0; i < 4; i++) {
			channelFilter[i].rise = 0.01f;
			channelFilter[i].fall = 0.01f;
		}
	}

	void step() override {
		// Determine current channel
		if (clockTrigger.process(inputs[CLOCK_INPUT].value / 2.f)) {
			channel++;
		}
		if (resetTrigger.process(inputs[RESET_INPUT].value / 2.f)) {
			channel = 0;
		}
		int channels = 4 - (int) params[CHANNELS_PARAM].value;
		channel %= channels;

		// Filter channels
		for (int i = 0; i < 4; i++) {
			channelFilter[i].process(channel == i ? 1.0f : 0.0f);
		}

		// Set outputs
		if (TYPE == 1) {
			float out = inputs[IN_INPUT + 0].value;
			for (int i = 0; i < 4; i++) {
				outputs[OUT_OUTPUT + i].value = channelFilter[i].out * out;
			}
		}
		else {
			float out = 0.0f;
			for (int i = 0; i < 4; i++) {
				out += channelFilter[i].out * inputs[IN_INPUT + i].value;
			}
			outputs[OUT_OUTPUT + 0].value = out;
		}

		// Set lights
		for (int i = 0; i < 4; i++) {
			lights[CHANNEL_LIGHT + i].setBrightness(channelFilter[i].out);
		}
	}
};


struct SequentialSwitch1Widget : ModuleWidget {
	SequentialSwitch1Widget(SequentialSwitch<1> *module);
};

SequentialSwitch1Widget::SequentialSwitch1Widget(SequentialSwitch<1> *module) : ModuleWidget(module) {
	typedef SequentialSwitch<1> TSequentialSwitch;
	setPanel(SVG::load(assetPlugin(plugin, "res/SequentialSwitch1.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(ParamWidget::create<CKSSThree>(mm2px(Vec(5.24619, 46.9153)), module, TSequentialSwitch::CHANNELS_PARAM, 0.0f, 2.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 17.694)), Port::INPUT, module, TSequentialSwitch::CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 32.1896)), Port::INPUT, module, TSequentialSwitch::RESET_INPUT));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51536, 62.8096)), Port::INPUT, module, TSequentialSwitch::IN_INPUT + 0));

	addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.51536, 77.8095)), Port::OUTPUT, module, TSequentialSwitch::OUT_OUTPUT + 0));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 87.8113)), Port::OUTPUT, module, TSequentialSwitch::OUT_OUTPUT + 1));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 97.809)), Port::OUTPUT, module, TSequentialSwitch::OUT_OUTPUT + 2));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 107.809)), Port::OUTPUT, module, TSequentialSwitch::OUT_OUTPUT + 3));

	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(mm2px(Vec(10.8203, 77.7158)), module, TSequentialSwitch::CHANNEL_LIGHT + 0));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(mm2px(Vec(10.8203, 87.7163)), module, TSequentialSwitch::CHANNEL_LIGHT + 1));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(mm2px(Vec(10.8203, 97.7167)), module, TSequentialSwitch::CHANNEL_LIGHT + 2));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(mm2px(Vec(10.8203, 107.716)), module, TSequentialSwitch::CHANNEL_LIGHT + 3));
}


RACK_PLUGIN_MODEL_INIT(Fundamental, SequentialSwitch1) {
   Model *modelSequentialSwitch1 = Model::create<SequentialSwitch<1>, SequentialSwitch1Widget>("Fundamental", "SequentialSwitch1", "Sequential Switch 1", UTILITY_TAG);
   return modelSequentialSwitch1;
}


struct SequentialSwitch2Widget : ModuleWidget {
	SequentialSwitch2Widget(SequentialSwitch<2> *module);
};

SequentialSwitch2Widget::SequentialSwitch2Widget(SequentialSwitch<2> *module) : ModuleWidget(module) {
	typedef SequentialSwitch<2> TSequentialSwitch;
	setPanel(SVG::load(assetPlugin(plugin, "res/SequentialSwitch2.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(ParamWidget::create<CKSSThree>(mm2px(Vec(5.24619, 46.9153)), module, TSequentialSwitch::CHANNELS_PARAM, 0.0f, 2.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 17.694)), Port::INPUT, module, TSequentialSwitch::CLOCK_INPUT));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 32.191)), Port::INPUT, module, TSequentialSwitch::RESET_INPUT));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 62.811)), Port::INPUT, module, TSequentialSwitch::IN_INPUT + 0));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 72.8114)), Port::INPUT, module, TSequentialSwitch::IN_INPUT + 1));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 82.8091)), Port::INPUT, module, TSequentialSwitch::IN_INPUT + 2));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 92.8109)), Port::INPUT, module, TSequentialSwitch::IN_INPUT + 3));

	addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 107.622)), Port::OUTPUT, module, TSequentialSwitch::OUT_OUTPUT + 0));

	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(mm2px(Vec(10.7321, 62.6277)), module, TSequentialSwitch::CHANNEL_LIGHT + 0));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(mm2px(Vec(10.7321, 72.6281)), module, TSequentialSwitch::CHANNEL_LIGHT + 1));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(mm2px(Vec(10.7321, 82.6285)), module, TSequentialSwitch::CHANNEL_LIGHT + 2));
	addChild(ModuleLightWidget::create<TinyLight<GreenLight>>(mm2px(Vec(10.7321, 92.6276)), module, TSequentialSwitch::CHANNEL_LIGHT + 3));
}


RACK_PLUGIN_MODEL_INIT(Fundamental, SequentialSwitch2) {
   Model *modelSequentialSwitch2 = Model::create<SequentialSwitch<2>, SequentialSwitch2Widget>("Fundamental", "SequentialSwitch2", "Sequential Switch 2", UTILITY_TAG);
   return modelSequentialSwitch2;
}
