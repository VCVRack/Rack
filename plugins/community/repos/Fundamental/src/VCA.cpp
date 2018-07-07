#include "Fundamental.hpp"


struct VCA : Module {
	enum ParamIds {
		LEVEL1_PARAM,
		LEVEL2_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		EXP1_INPUT,
		LIN1_INPUT,
		IN1_INPUT,
		EXP2_INPUT,
		LIN2_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};

	VCA() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};


static void stepChannel(Input &in, Param &level, Input &lin, Input &exp, Output &out) {
	float v = in.value * level.value;
	if (lin.active)
		v *= clamp(lin.value / 10.0f, 0.0f, 1.0f);
	const float expBase = 50.0f;
	if (exp.active)
		v *= rescale(powf(expBase, clamp(exp.value / 10.0f, 0.0f, 1.0f)), 1.0f, expBase, 0.0f, 1.0f);
   v += 100.0f;
   v -= 100.0f;
   out.value = v;
}

void VCA::step() {
	stepChannel(inputs[IN1_INPUT], params[LEVEL1_PARAM], inputs[LIN1_INPUT], inputs[EXP1_INPUT], outputs[OUT1_OUTPUT]);
	stepChannel(inputs[IN2_INPUT], params[LEVEL2_PARAM], inputs[LIN2_INPUT], inputs[EXP2_INPUT], outputs[OUT2_OUTPUT]);
}


struct VCAWidget : ModuleWidget {
	VCAWidget(VCA *module);
};

VCAWidget::VCAWidget(VCA *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/VCA.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 57), module, VCA::LEVEL1_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(27, 222), module, VCA::LEVEL2_PARAM, 0.0f, 1.0f, 0.5f));

	addInput(Port::create<PJ301MPort>(Vec(11, 113), Port::INPUT, module, VCA::EXP1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(54, 113), Port::INPUT, module, VCA::LIN1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(11, 156), Port::INPUT, module, VCA::IN1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(11, 276), Port::INPUT, module, VCA::EXP2_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(54, 276), Port::INPUT, module, VCA::LIN2_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(11, 320), Port::INPUT, module, VCA::IN2_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(54, 156), Port::OUTPUT, module, VCA::OUT1_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(54, 320), Port::OUTPUT, module, VCA::OUT2_OUTPUT));
}


RACK_PLUGIN_MODEL_INIT(Fundamental, VCA) {
   Model *modelVCA = Model::create<VCA, VCAWidget>("Fundamental", "VCA", "VCA", AMPLIFIER_TAG, DUAL_TAG);
   return modelVCA;
}
