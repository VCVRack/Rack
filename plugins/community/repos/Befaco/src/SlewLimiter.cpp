#include "Befaco.hpp"


struct SlewLimiter : Module {
	enum ParamIds {
		SHAPE_PARAM,
		RISE_PARAM,
		FALL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		RISE_INPUT,
		FALL_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

	float out = 0.0;

	SlewLimiter() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS) {}
	void step() override;
};


void ::SlewLimiter::step() {
	float in = inputs[IN_INPUT].value;
	float shape = params[SHAPE_PARAM].value;

	// minimum and maximum slopes in volts per second
	const float slewMin = 0.1;
	const float slewMax = 10000.0;
	// Amount of extra slew per voltage difference
	const float shapeScale = 1/10.0;

	// Rise
	if (in > out) {
		float rise = inputs[RISE_INPUT].value / 10.0 + params[RISE_PARAM].value;
		float slew = slewMax * powf(slewMin / slewMax, rise);
		out += slew * crossfade(1.0f, shapeScale * (in - out), shape) * engineGetSampleTime();
		if (out > in)
			out = in;
	}
	// Fall
	else if (in < out) {
		float fall = inputs[FALL_INPUT].value / 10.0 + params[FALL_PARAM].value;
		float slew = slewMax * powf(slewMin / slewMax, fall);
		out -= slew * crossfade(1.0f, shapeScale * (out - in), shape) * engineGetSampleTime();
		if (out < in)
			out = in;
	}

	outputs[OUT_OUTPUT].value = out;
}


struct SlewLimiterWidget : ModuleWidget {
	SlewLimiterWidget(SlewLimiter *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/SlewLimiter.svg")));

		addChild(Widget::create<Knurlie>(Vec(15, 0)));
		addChild(Widget::create<Knurlie>(Vec(15, 365)));

		addParam(ParamWidget::create<Davies1900hWhiteKnob>(Vec(27, 39), module, ::SlewLimiter::SHAPE_PARAM, 0.0, 1.0, 0.0));

		addParam(ParamWidget::create<BefacoSlidePot>(Vec(15, 102), module, ::SlewLimiter::RISE_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<BefacoSlidePot>(Vec(60, 102), module, ::SlewLimiter::FALL_PARAM, 0.0, 1.0, 0.0));

		addInput(Port::create<PJ301MPort>(Vec(10, 273), Port::INPUT, module, ::SlewLimiter::RISE_INPUT));
		addInput(Port::create<PJ301MPort>(Vec(55, 273), Port::INPUT, module, ::SlewLimiter::FALL_INPUT));

		addInput(Port::create<PJ301MPort>(Vec(10, 323), Port::INPUT, module, ::SlewLimiter::IN_INPUT));
		addOutput(Port::create<PJ301MPort>(Vec(55, 323), Port::OUTPUT, module, ::SlewLimiter::OUT_OUTPUT));
	}
};


RACK_PLUGIN_MODEL_INIT(Befaco, SlewLimiter) {
   Model *modelSlewLimiter = Model::create<SlewLimiter, SlewLimiterWidget>("Befaco", "SlewLimiter", "Slew Limiter", SLEW_LIMITER_TAG, ENVELOPE_FOLLOWER_TAG);
   return modelSlewLimiter;
}
