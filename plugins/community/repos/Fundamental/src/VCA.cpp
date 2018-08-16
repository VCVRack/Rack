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

	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(ParamWidget::create<RoundLargeBlackKnob>(mm2px(Vec(6.35, 19.11753)), module, VCA::LEVEL1_PARAM, 0.0, 1.0, 0.0));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(mm2px(Vec(6.35, 74.80544)), module, VCA::LEVEL2_PARAM, 0.0, 1.0, 0.0));

	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.5907, 38.19371)), Port::INPUT, module, VCA::EXP1_INPUT));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(14.59752, 38.19371)), Port::INPUT, module, VCA::LIN1_INPUT));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.5907, 52.80642)), Port::INPUT, module, VCA::IN1_INPUT));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.5907, 93.53435)), Port::INPUT, module, VCA::EXP2_INPUT));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(14.59752, 93.53435)), Port::INPUT, module, VCA::LIN2_INPUT));
	addInput(Port::create<PJ301MPort>(mm2px(Vec(2.5907, 108.14706)), Port::INPUT, module, VCA::IN2_INPUT));

	addOutput(Port::create<PJ301MPort>(mm2px(Vec(14.59752, 52.80642)), Port::OUTPUT, module, VCA::OUT1_OUTPUT));
	addOutput(Port::create<PJ301MPort>(mm2px(Vec(14.59752, 108.14706)), Port::OUTPUT, module, VCA::OUT2_OUTPUT));
}

struct VCA_1 : Module {
	enum ParamIds {
		LEVEL_PARAM,
		EXP_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		CV_INPUT,
		IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	float lastCv = 0.f;

	VCA_1() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

	void step() override {
		float cv = 1.f;
		if (inputs[CV_INPUT].active)
			cv = fmaxf(inputs[CV_INPUT].value / 10.f, 0.f);
		if ((int) params[EXP_PARAM].value == 0)
			cv = powf(cv, 4.f);
		lastCv = cv;
		outputs[OUT_OUTPUT].value = inputs[IN_INPUT].value * params[LEVEL_PARAM].value * cv;
	}
};


struct VCA_1VUKnob : Knob {
	VCA_1VUKnob() {
		box.size = mm2px(Vec(10, 46));
	}

	void draw(NVGcontext *vg) override {
		nvgBeginPath(vg);
		nvgRoundedRect(vg, 0, 0, box.size.x, box.size.y, 2.0);
		nvgFillColor(vg, nvgRGB(0, 0, 0));
		nvgFill(vg);

		VCA_1 *module = dynamic_cast<VCA_1*>(this->module);

		const int segs = 25;
		const Vec margin = Vec(4, 4);
		Rect r = box.zeroPos().shrink(margin);

		for (int i = 0; i < segs; i++) {
			float segValue = clamp(value * segs - (segs - i - 1), 0.f, 1.f);
			float amplitude = value * module->lastCv;
			float segAmplitude = clamp(amplitude * segs - (segs - i - 1), 0.f, 1.f);
			nvgBeginPath(vg);
			nvgRect(vg, r.pos.x, r.pos.y + r.size.y / segs * i + 0.5,
				r.size.x, r.size.y / segs - 1.0);
			if (segValue > 0.f) {
				nvgFillColor(vg, colorAlpha(nvgRGBf(0.33, 0.33, 0.33), segValue));
				nvgFill(vg);
			}
			if (segAmplitude > 0.f) {
				nvgFillColor(vg, colorAlpha(COLOR_GREEN, segAmplitude));
				nvgFill(vg);
			}
		}
	}
};


struct VCA_1Widget : ModuleWidget {
	VCA_1Widget(VCA_1 *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/VCA-1.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(ParamWidget::create<VCA_1VUKnob>(mm2px(Vec(2.62103, 12.31692)), module, VCA_1::LEVEL_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<CKSS>(mm2px(Vec(5.24619, 79.9593)), module, VCA_1::EXP_PARAM, 0.0, 1.0, 1.0));

		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51261, 60.4008)), Port::INPUT, module, VCA_1::CV_INPUT));
		addInput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 97.74977)), Port::INPUT, module, VCA_1::IN_INPUT));

		addOutput(Port::create<PJ301MPort>(mm2px(Vec(3.51398, 108.64454)), Port::OUTPUT, module, VCA_1::OUT_OUTPUT));
	}
};

RACK_PLUGIN_MODEL_INIT(Fundamental, VCA_1) {
   Model *modelVCA_1 = Model::create<VCA_1, VCA_1Widget>("Fundamental", "VCA-1", "VCA-1", AMPLIFIER_TAG);
   return modelVCA_1;
}

RACK_PLUGIN_MODEL_INIT(Fundamental, VCA) {
   Model *modelVCA = Model::create<VCA, VCAWidget>("Fundamental", "VCA", "VCA-2", AMPLIFIER_TAG, DUAL_TAG);
   return modelVCA;
}
