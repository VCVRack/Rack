#include "AudibleInstruments.hpp"
#include <string.h>


struct Shades : Module {
	enum ParamIds {
		GAIN1_PARAM,
		GAIN2_PARAM,
		GAIN3_PARAM,
		MODE1_PARAM,
		MODE2_PARAM,
		MODE3_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUT3_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		OUT1_POS_LIGHT, OUT1_NEG_LIGHT,
		OUT2_POS_LIGHT, OUT2_NEG_LIGHT,
		OUT3_POS_LIGHT, OUT3_NEG_LIGHT,
		NUM_LIGHTS
	};

	Shades() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void Shades::step() {
	float out = 0.0;

	for (int i = 0; i < 3; i++) {
		float in = inputs[IN1_INPUT + i].normalize(5.0);
		if ((int)params[MODE1_PARAM + i].value == 1) {
			// attenuverter
			in *= 2.0 * params[GAIN1_PARAM + i].value - 1.0;
		}
		else {
			// attenuator
			in *= params[GAIN1_PARAM + i].value;
		}
		out += in;
		lights[OUT1_POS_LIGHT + 2*i].setBrightnessSmooth(fmaxf(0.0, out / 5.0));
		lights[OUT1_NEG_LIGHT + 2*i].setBrightnessSmooth(fmaxf(0.0, -out / 5.0));
		if (outputs[OUT1_OUTPUT + i].active) {
			outputs[OUT1_OUTPUT + i].value = out;
			out = 0.0;
		}
	}
}


struct ShadesWidget : ModuleWidget {
	ShadesWidget(Shades *module) : ModuleWidget(module) {
#ifdef USE_VST2
		setPanel(SVG::load(assetStaticPlugin("AudibleInstruments", "res/Shades.svg")));
#else
		setPanel(SVG::load(assetPlugin(plugin, "res/Shades.svg")));
#endif // USE_VST2

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

		addParam(ParamWidget::create<Rogan1PSRed>(Vec(40, 40), module, Shades::GAIN1_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Rogan1PSWhite>(Vec(40, 106), module, Shades::GAIN2_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Rogan1PSGreen>(Vec(40, 172), module, Shades::GAIN3_PARAM, 0.0, 1.0, 0.5));

		addParam(ParamWidget::create<CKSS>(Vec(10, 51), module, Shades::MODE1_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<CKSS>(Vec(10, 117), module, Shades::MODE2_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<CKSS>(Vec(10, 183), module, Shades::MODE3_PARAM, 0.0, 1.0, 1.0));

		addInput(Port::create<PJ301MPort>(Vec(9, 245), Port::INPUT, module, Shades::IN1_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(9, 281), Port::INPUT, module, Shades::IN2_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(9, 317), Port::INPUT, module, Shades::IN3_INPUT));

		addOutput(Port::create<PJ301MPort>(Vec(56, 245), Port::OUTPUT, module, Shades::OUT1_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(56, 281), Port::OUTPUT, module, Shades::OUT2_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(56, 317), Port::OUTPUT, module, Shades::OUT3_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(41, 254), module, Shades::OUT1_POS_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(41, 290), module, Shades::OUT2_POS_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(41, 326), module, Shades::OUT3_POS_LIGHT));
	}
};


RACK_PLUGIN_MODEL_INIT(AudibleInstruments, Shades) {
   Model *modelShades = Model::create<Shades, ShadesWidget>("Audible Instruments", "Shades", "Mixer", MIXER_TAG);
   return modelShades;
}

