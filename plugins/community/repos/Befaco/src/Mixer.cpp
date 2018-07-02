#include "Befaco.hpp"


struct Mixer : Module {
	enum ParamIds {
		CH1_PARAM,
		CH2_PARAM,
		CH3_PARAM,
		CH4_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,
		IN4_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		OUT_POS_LIGHT,
		OUT_NEG_LIGHT,
		NUM_LIGHTS
	};

	Mixer() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void Mixer::step() {
	float in1 = inputs[IN1_INPUT].value * params[CH1_PARAM].value;
	float in2 = inputs[IN2_INPUT].value * params[CH2_PARAM].value;
	float in3 = inputs[IN3_INPUT].value * params[CH3_PARAM].value;
	float in4 = inputs[IN4_INPUT].value * params[CH4_PARAM].value;

	float out = in1 + in2 + in3 + in4;
	outputs[OUT1_OUTPUT].value = out;
	outputs[OUT2_OUTPUT].value = -out;
	lights[OUT_POS_LIGHT].setBrightnessSmooth(out / 5.0);
	lights[OUT_NEG_LIGHT].setBrightnessSmooth(-out / 5.0);
}


struct MixerWidget : ModuleWidget {
	MixerWidget(Mixer *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Mixer.svg")));

		addChild(Widget::create<Knurlie>(Vec(15, 0)));
		addChild(Widget::create<Knurlie>(Vec(15, 365)));

		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(19, 32), module, Mixer::CH1_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(19, 85), module, Mixer::CH2_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(19, 137), module, Mixer::CH3_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(19, 190), module, Mixer::CH4_PARAM, 0.0, 1.0, 0.0));

		addInput(Port::create<PJ301MPort>(Vec(7, 242), Port::INPUT, module, Mixer::IN1_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(43, 242), Port::INPUT, module, Mixer::IN2_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(7, 281), Port::INPUT, module, Mixer::IN3_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(43, 281), Port::INPUT, module, Mixer::IN4_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(7, 324), Port::OUTPUT, module, Mixer::OUT1_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(43, 324), Port::OUTPUT, module, Mixer::OUT2_OUTPUT));

		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(32.7, 310), module, Mixer::OUT_POS_LIGHT));
	}
};


RACK_PLUGIN_MODEL_INIT(Befaco, Mixer) {
   Model *modelMixer = Model::create<Mixer, MixerWidget>("Befaco", "Mixer", "Mixer", MIXER_TAG);
   return modelMixer;
}
