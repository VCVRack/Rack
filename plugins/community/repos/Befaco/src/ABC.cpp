#include "Befaco.hpp"
#include "dsp/functions.hpp"


struct ABC : Module {
	enum ParamIds {
		B1_LEVEL_PARAM,
		C1_LEVEL_PARAM,
		B2_LEVEL_PARAM,
		C2_LEVEL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		A1_INPUT,
		B1_INPUT,
		C1_INPUT,
		A2_INPUT,
		B2_INPUT,
		C2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		OUT1_POS_LIGHT,
		OUT1_NEG_LIGHT,
		OUT2_POS_LIGHT,
		OUT2_NEG_LIGHT,
		NUM_LIGHTS
	};

	ABC() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


static float clip(float x) {
	x = clamp(x, -2.0f, 2.0f);
	return x / powf(1.0 + powf(x, 24.0), 1/24.0);
}

void ABC::step() {
	float a1 = inputs[A1_INPUT].value;
	float b1 = inputs[B1_INPUT].normalize(5.0) * 2.0*exponentialBipolar(80.0, params[B1_LEVEL_PARAM].value);
	float c1 = inputs[C1_INPUT].normalize(10.0) * exponentialBipolar(80.0, params[C1_LEVEL_PARAM].value);
	float out1 = a1 * b1 / 5.0 + c1;

	float a2 = inputs[A2_INPUT].value;
	float b2 = inputs[B2_INPUT].normalize(5.0) * 2.0*exponentialBipolar(80.0, params[B2_LEVEL_PARAM].value);
	float c2 = inputs[C2_INPUT].normalize(10.0) * exponentialBipolar(80.0, params[C2_LEVEL_PARAM].value);
	float out2 = a2 * b2 / 5.0 + c2;

	// Set outputs
	if (outputs[OUT1_OUTPUT].active) {
		outputs[OUT1_OUTPUT].value = clip(out1 / 10.0) * 10.0;
	}
	else {
		out2 += out1;
	}
	if (outputs[OUT2_OUTPUT].active) {
		outputs[OUT2_OUTPUT].value = clip(out2 / 10.0) * 10.0;
	}

	// Lights
	lights[OUT1_POS_LIGHT].value = fmaxf(0.0, out1 / 5.0);
	lights[OUT1_NEG_LIGHT].value = fmaxf(0.0, -out1 / 5.0);
	lights[OUT2_POS_LIGHT].value = fmaxf(0.0, out2 / 5.0);
	lights[OUT2_NEG_LIGHT].value = fmaxf(0.0, -out2 / 5.0);
}


struct ABCWidget : ModuleWidget {
	ABCWidget(ABC *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/ABC.svg")));

		addChild(Widget::create<Knurlie>(Vec(15, 0)));
		addChild(Widget::create<Knurlie>(Vec(15, 365)));

		addParam(ParamWidget::create<Davies1900hRedKnob>(Vec(45, 37), module, ABC::B1_LEVEL_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(45, 107), module, ABC::C1_LEVEL_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Davies1900hRedKnob>(Vec(45, 204), module, ABC::B2_LEVEL_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(45, 274), module, ABC::C2_LEVEL_PARAM, -1.0, 1.0, 0.0));

		addInput(Port::create<PJ301MPort>(Vec(7, 28), Port::INPUT, module, ABC::A1_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(7, 70), Port::INPUT, module, ABC::B1_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(7, 112), Port::INPUT, module, ABC::C1_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(7, 154), Port::OUTPUT, module, ABC::OUT1_OUTPUT));
		addInput(Port::create<PJ301MPort>(Vec(7, 195), Port::INPUT, module, ABC::A2_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(7, 237), Port::INPUT, module, ABC::B2_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(7, 279), Port::INPUT, module, ABC::C2_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(7, 321), Port::OUTPUT, module, ABC::OUT2_OUTPUT));

		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(37, 162), module, ABC::OUT1_POS_LIGHT));
		addChild(ModuleLightWidget::create<MediumLight<GreenRedLight>>(Vec(37, 329), module, ABC::OUT2_POS_LIGHT));
	}
};


RACK_PLUGIN_MODEL_INIT(Befaco, ABC) {
   Model *modelABC = Model::create<ABC, ABCWidget>("Befaco", "ABC", "A*B+C", RING_MODULATOR_TAG, ATTENUATOR_TAG, DUAL_TAG);
   return modelABC;
}
