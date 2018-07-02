#include "AudibleInstruments.hpp"


struct Links : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		A1_INPUT,
		B1_INPUT,
		B2_INPUT,
		C1_INPUT,
		C2_INPUT,
		C3_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		A1_OUTPUT,
		A2_OUTPUT,
		A3_OUTPUT,
		B1_OUTPUT,
		B2_OUTPUT,
		C1_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		A_POS_LIGHT, A_NEG_LIGHT,
		B_POS_LIGHT, B_NEG_LIGHT,
		C_POS_LIGHT, C_NEG_LIGHT,
		NUM_LIGHTS
	};

	Links() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void Links::step() {
	float inA = inputs[A1_INPUT].value;
	float inB = inputs[B1_INPUT].value + inputs[B2_INPUT].value;
	float inC = inputs[C1_INPUT].value + inputs[C2_INPUT].value + inputs[C3_INPUT].value;

	outputs[A1_OUTPUT].value = inA;
	outputs[A2_OUTPUT].value = inA;
	outputs[A3_OUTPUT].value = inA;
	outputs[B1_OUTPUT].value = inB;
	outputs[B2_OUTPUT].value = inB;
	outputs[C1_OUTPUT].value = inC;

	lights[A_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, inA / 5.0));
	lights[A_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -inA / 5.0));
	lights[B_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, inB / 5.0));
	lights[B_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -inB / 5.0));
	lights[C_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0, inC / 5.0));
	lights[C_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0, -inC / 5.0));
}


struct LinksWidget : ModuleWidget {
	LinksWidget(Links *module) : ModuleWidget(module) {
#ifdef USE_VST2
		setPanel(SVG::load(assetStaticPlugin("AudibleInstruments", "res/Links.svg")));
#else
		setPanel(SVG::load(assetPlugin(plugin, "res/Links.svg")));
#endif // USE_VST2

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

		addInput(Port::create<PJ301MPort>(Vec(4, 75), Port::INPUT, module, Links::A1_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(31, 75), Port::OUTPUT, module, Links::A1_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(4, 113), Port::OUTPUT, module, Links::A2_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(31, 113), Port::OUTPUT, module, Links::A3_OUTPUT));

		addInput(Port::create<PJ301MPort>(Vec(4, 177), Port::INPUT, module, Links::B1_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(31, 177), Port::INPUT, module, Links::B2_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(4, 214), Port::OUTPUT, module, Links::B1_OUTPUT));
		addOutput(Port::create<PJ301MPort>(Vec(31, 214), Port::OUTPUT, module, Links::B2_OUTPUT));

		addInput(Port::create<PJ301MPort>(Vec(4, 278), Port::INPUT, module, Links::C1_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(31, 278), Port::INPUT, module, Links::C2_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(4, 316), Port::INPUT, module, Links::C3_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(31, 316), Port::OUTPUT, module, Links::C1_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(26, 59), module, Links::A_POS_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(26, 161), module, Links::B_POS_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(26, 262), module, Links::C_POS_LIGHT));
	}
};

RACK_PLUGIN_MODEL_INIT(AudibleInstruments, Links) {
   Model *modelLinks = Model::create<Links, LinksWidget>("Audible Instruments", "Links", "Multiples", MULTIPLE_TAG, MIXER_TAG);
   return modelLinks;
}

