#include "Fundamental.hpp"
#include "dsp/digital.hpp"


struct LowFrequencyOscillator {
	float phase = 0.0f;
	float pw = 0.5f;
	float freq = 1.0f;
	bool offset = false;
	bool invert = false;
	SchmittTrigger resetTrigger;

	LowFrequencyOscillator() {}
	void setPitch(float pitch) {
		pitch = fminf(pitch, 10.0f);
		freq = powf(2.0f, pitch);
	}
	void setPulseWidth(float pw_) {
		const float pwMin = 0.01f;
		pw = clamp(pw_, pwMin, 1.0f - pwMin);
	}
	void setReset(float reset) {
		if (resetTrigger.process(reset / 0.01f)) {
			phase = 0.0f;
		}
	}
	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;
	}
	float sin() {
		if (offset)
			return 1.0f - cosf(2*M_PI * phase) * (invert ? -1.0f : 1.0f);
		else
			return sinf(2*M_PI * phase) * (invert ? -1.0f : 1.0f);
	}
	float tri(float x) {
		return 4.0f * fabsf(x - roundf(x));
	}
	float tri() {
		if (offset)
			return tri(invert ? phase - 0.5f : phase);
		else
			return -1.0f + tri(invert ? phase - 0.25f : phase - 0.75f);
	}
	float saw(float x) {
		return 2.0f * (x - roundf(x));
	}
	float saw() {
		if (offset)
			return invert ? 2.0f * (1.0f - phase) : 2.0f * phase;
		else
			return saw(phase) * (invert ? -1.0f : 1.0f);
	}
	float sqr() {
		float sqr = (phase < pw) ^ invert ? 1.0f : -1.0f;
		return offset ? sqr + 1.0f : sqr;
	}
	float light() {
		return sinf(2*M_PI * phase);
	}
};


struct LFO : Module {
	enum ParamIds {
		OFFSET_PARAM,
		INVERT_PARAM,
		FREQ_PARAM,
		FM1_PARAM,
		FM2_PARAM,
		PW_PARAM,
		PWM_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FM1_INPUT,
		FM2_INPUT,
		RESET_INPUT,
		PW_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SIN_OUTPUT,
		TRI_OUTPUT,
		SAW_OUTPUT,
		SQR_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PHASE_POS_LIGHT,
		PHASE_NEG_LIGHT,
		NUM_LIGHTS
	};

	LowFrequencyOscillator oscillator;

	LFO() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void LFO::step() {
	oscillator.setPitch(params[FREQ_PARAM].value + params[FM1_PARAM].value * inputs[FM1_INPUT].value + params[FM2_PARAM].value * inputs[FM2_INPUT].value);
	oscillator.setPulseWidth(params[PW_PARAM].value + params[PWM_PARAM].value * inputs[PW_INPUT].value / 10.0f);
	oscillator.offset = (params[OFFSET_PARAM].value > 0.0f);
	oscillator.invert = (params[INVERT_PARAM].value <= 0.0f);
	oscillator.step(engineGetSampleTime());
	oscillator.setReset(inputs[RESET_INPUT].value);

	outputs[SIN_OUTPUT].value = 5.0f * oscillator.sin();
	outputs[TRI_OUTPUT].value = 5.0f * oscillator.tri();
	outputs[SAW_OUTPUT].value = 5.0f * oscillator.saw();
	outputs[SQR_OUTPUT].value = 5.0f * oscillator.sqr();

	lights[PHASE_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0f, oscillator.light()));
	lights[PHASE_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -oscillator.light()));
}


struct LFOWidget : ModuleWidget {
	LFOWidget(LFO *module);
};

LFOWidget::LFOWidget(LFO *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/LFO-1.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<CKSS>(Vec(15, 77), module, LFO::OFFSET_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<CKSS>(Vec(119, 77), module, LFO::INVERT_PARAM, 0.0f, 1.0f, 1.0f));

	addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(47, 61), module, LFO::FREQ_PARAM, -8.0f, 10.0f, 1.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(23, 143), module, LFO::FM1_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(91, 143), module, LFO::PW_PARAM, 0.0f, 1.0f, 0.5f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(23, 208), module, LFO::FM2_PARAM, 0.0f, 1.0f, 0.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(91, 208), module, LFO::PWM_PARAM, 0.0f, 1.0f, 0.0f));

	addInput(Port::create<PJ301MPort>(Vec(11, 276), Port::INPUT, module, LFO::FM1_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(45, 276), Port::INPUT, module, LFO::FM2_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(80, 276), Port::INPUT, module, LFO::RESET_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(114, 276), Port::INPUT, module, LFO::PW_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(11, 320), Port::OUTPUT, module, LFO::SIN_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(45, 320), Port::OUTPUT, module, LFO::TRI_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(80, 320), Port::OUTPUT, module, LFO::SAW_OUTPUT));
	addOutput(Port::create<PJ301MPort>(Vec(114, 320), Port::OUTPUT, module, LFO::SQR_OUTPUT));

	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(99, 42.5f), module, LFO::PHASE_POS_LIGHT));
}


RACK_PLUGIN_MODEL_INIT(Fundamental, LFO) {
   Model *modelLFO = Model::create<LFO, LFOWidget>("Fundamental", "LFO", "LFO-1", LFO_TAG);
   return modelLFO;
}


struct LFO2 : Module {
	enum ParamIds {
		OFFSET_PARAM,
		INVERT_PARAM,
		FREQ_PARAM,
		WAVE_PARAM,
		FM_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		FM_INPUT,
		RESET_INPUT,
		WAVE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		INTERP_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		PHASE_POS_LIGHT,
		PHASE_NEG_LIGHT,
		NUM_LIGHTS
	};

	LowFrequencyOscillator oscillator;

	LFO2() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;
};


void LFO2::step() {
	oscillator.setPitch(params[FREQ_PARAM].value + params[FM_PARAM].value * inputs[FM_INPUT].value);
	oscillator.offset = (params[OFFSET_PARAM].value > 0.0f);
	oscillator.invert = (params[INVERT_PARAM].value <= 0.0f);
	oscillator.step(engineGetSampleTime());
	oscillator.setReset(inputs[RESET_INPUT].value);

	float wave = params[WAVE_PARAM].value + inputs[WAVE_INPUT].value;
	wave = clamp(wave, 0.0f, 3.0f);
	float interp;
	if (wave < 1.0f)
		interp = crossfade(oscillator.sin(), oscillator.tri(), wave);
	else if (wave < 2.0f)
		interp = crossfade(oscillator.tri(), oscillator.saw(), wave - 1.0f);
	else
		interp = crossfade(oscillator.saw(), oscillator.sqr(), wave - 2.0f);
	outputs[INTERP_OUTPUT].value = 5.0f * interp;

	lights[PHASE_POS_LIGHT].setBrightnessSmooth(fmaxf(0.0f, oscillator.light()));
	lights[PHASE_NEG_LIGHT].setBrightnessSmooth(fmaxf(0.0f, -oscillator.light()));
}


struct LFO2Widget : ModuleWidget {
	LFO2Widget(LFO2 *module);
};

LFO2Widget::LFO2Widget(LFO2 *module) : ModuleWidget(module) {
	setPanel(SVG::load(assetPlugin(plugin, "res/LFO-2.svg")));

	addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 0)));
	addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
	addChild(Widget::create<ScrewSilver>(Vec(box.size.x-30, 365)));

	addParam(ParamWidget::create<CKSS>(Vec(62, 150), module, LFO2::OFFSET_PARAM, 0.0f, 1.0f, 1.0f));
	addParam(ParamWidget::create<CKSS>(Vec(62, 215), module, LFO2::INVERT_PARAM, 0.0f, 1.0f, 1.0f));

	addParam(ParamWidget::create<RoundHugeBlackKnob>(Vec(18, 60), module, LFO2::FREQ_PARAM, -8.0f, 10.0f, 1.0f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(11, 142), module, LFO2::WAVE_PARAM, 0.0f, 3.0f, 1.5f));
	addParam(ParamWidget::create<RoundLargeBlackKnob>(Vec(11, 207), module, LFO2::FM_PARAM, 0.0f, 1.0f, 0.5f));

	addInput(Port::create<PJ301MPort>(Vec(11, 276), Port::INPUT, module, LFO2::FM_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(54, 276), Port::INPUT, module, LFO2::RESET_INPUT));
	addInput(Port::create<PJ301MPort>(Vec(11, 319), Port::INPUT, module, LFO2::WAVE_INPUT));

	addOutput(Port::create<PJ301MPort>(Vec(54, 319), Port::OUTPUT, module, LFO2::INTERP_OUTPUT));

	addChild(ModuleLightWidget::create<SmallLight<GreenRedLight>>(Vec(68, 42.5f), module, LFO2::PHASE_POS_LIGHT));
}


RACK_PLUGIN_MODEL_INIT(Fundamental, LFO2) {
   Model *modelLFO2 = Model::create<LFO2, LFO2Widget>("Fundamental", "LFO2", "LFO-2", LFO_TAG);
   return modelLFO2;
}
