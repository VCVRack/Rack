
#include "FMOp.hpp"
#include "dsp/pitch.hpp"

#define LINEAR_LEVEL "linearLevel"

void FMOp::onReset() {
	_steps = modulationSteps;
	_envelope.reset();
	_gateTrigger.reset();
}

void FMOp::onSampleRateChange() {
	_steps = modulationSteps;
	float sampleRate = engineGetSampleRate();
	_envelope.setSampleRate(sampleRate);
	_phasor.setSampleRate(sampleRate);
	_decimator.setParams(sampleRate, oversample);
	_maxFrequency = 0.475f * sampleRate;
	_feedbackSL.setParams(sampleRate, 5.0f, 1.0f);
	_depthSL.setParams(sampleRate, 5.0f, 1.0f);
	_levelSL.setParams(sampleRate, 10.0f, 1.0f);
	_sustainSL.setParams(sampleRate, 1.0f, 1.0f);
}

json_t* FMOp::toJson() {
	json_t* root = json_object();
	json_object_set_new(root, LINEAR_LEVEL, json_boolean(_linearLevel));
	return root;
}

void FMOp::fromJson(json_t* root) {
	json_t* ll = json_object_get(root, LINEAR_LEVEL);
	if (ll) {
		_linearLevel = json_is_true(ll);
	}
}

void FMOp::step() {
	if (!outputs[AUDIO_OUTPUT].active) {
		lights[ENV_TO_LEVEL_LIGHT].value = params[ENV_TO_LEVEL_PARAM].value > 0.5f;
		lights[ENV_TO_FEEDBACK_LIGHT].value = params[ENV_TO_FEEDBACK_PARAM].value > 0.5f;
		lights[ENV_TO_DEPTH_LIGHT].value = params[ENV_TO_DEPTH_PARAM].value > 0.5f;
		return;
	}
	lights[ENV_TO_LEVEL_LIGHT].value = _levelEnvelopeOn;
	lights[ENV_TO_FEEDBACK_LIGHT].value = _feedbackEnvelopeOn;
	lights[ENV_TO_DEPTH_LIGHT].value = _depthEnvelopeOn;

	float pitchIn = 0.0f;
	if (inputs[PITCH_INPUT].active) {
		pitchIn = inputs[PITCH_INPUT].value;
	}
	float gateIn = 0.0f;
	if (inputs[GATE_INPUT].active) {
		gateIn = inputs[GATE_INPUT].value;
	}

	++_steps;
	if (_steps >= modulationSteps) {
		_steps = 0;

		float ratio = params[RATIO_PARAM].value;
		if (ratio < 0.0f) {
			ratio = std::max(1.0f + ratio, 0.01f);
		}
		else {
			ratio *= 9.0f;
			ratio += 1.0f;
		}
		float frequency = pitchIn;
		frequency += params[FINE_PARAM].value / 12.0f;
		frequency = cvToFrequency(frequency);
		frequency *= ratio;
		frequency = clamp(frequency, -_maxFrequency, _maxFrequency);
		_phasor.setFrequency(frequency / (float)oversample);

		bool levelEnvelopeOn = params[ENV_TO_LEVEL_PARAM].value > 0.5f;
		bool feedbackEnvelopeOn = params[ENV_TO_FEEDBACK_PARAM].value > 0.5f;
		bool depthEnvelopeOn = params[ENV_TO_DEPTH_PARAM].value > 0.5f;
		if (_levelEnvelopeOn != levelEnvelopeOn || _feedbackEnvelopeOn != feedbackEnvelopeOn || _depthEnvelopeOn != depthEnvelopeOn) {
			_levelEnvelopeOn = levelEnvelopeOn;
			_feedbackEnvelopeOn = feedbackEnvelopeOn;
			_depthEnvelopeOn = depthEnvelopeOn;
			bool envelopeOn = _levelEnvelopeOn || _feedbackEnvelopeOn || _depthEnvelopeOn;
			if (envelopeOn && !_envelopeOn) {
				_envelope.reset();
			}
			_envelopeOn = envelopeOn;
		}
		if (_envelopeOn) {
			float sustain = params[SUSTAIN_PARAM].value;
			if (inputs[SUSTAIN_INPUT].active) {
				sustain *= clamp(inputs[SUSTAIN_INPUT].value / 10.0f, 0.0f, 1.0f);
			}
			_envelope.setAttack(powf(params[ATTACK_PARAM].value, 2.0f) * 10.f);
			_envelope.setDecay(powf(params[DECAY_PARAM].value, 2.0f) * 10.f);
			_envelope.setSustain(_sustainSL.next(sustain));
			_envelope.setRelease(powf(params[RELEASE_PARAM].value, 2.0f) * 10.f);
		}

		_feedback = params[FEEDBACK_PARAM].value;
		if (inputs[FEEDBACK_INPUT].active) {
			_feedback *= clamp(inputs[FEEDBACK_INPUT].value / 10.0f, 0.0f, 1.0f);
		}

		_depth = params[DEPTH_PARAM].value;
		if (inputs[DEPTH_INPUT].active) {
			_depth *= clamp(inputs[DEPTH_INPUT].value / 10.0f, 0.0f, 1.0f);
		}

		_level = params[LEVEL_PARAM].value;
		if (inputs[LEVEL_INPUT].active) {
			_level *= clamp(inputs[LEVEL_INPUT].value / 10.0f, 0.0f, 1.0f);
		}
	}

	float envelope = 0.0f;
	if (_envelopeOn) {
		_gateTrigger.process(gateIn);
		_envelope.setGate(_gateTrigger.isHigh());
		envelope = _envelope.next();
	}

	float feedback = _feedbackSL.next(_feedback);
	if (_feedbackEnvelopeOn) {
		feedback *= envelope;
	}
	bool feedbackOn = feedback > 0.001f;

	float out = _levelSL.next(_level);
	if (_levelEnvelopeOn) {
		out *= envelope;
	}

	float offset = 0.0f;
	if (feedbackOn) {
		offset = feedback * _feedbackDelayedSample;
	}

	if (inputs[FM_INPUT].active) {
		float depth = _depthSL.next(_depth);
		if (_depthEnvelopeOn) {
			depth *= envelope;
		}
		offset += inputs[FM_INPUT].value * depth * 2.0f;
	}

	float sample = 0.0f;
	if (out > 0.0001f) {
		Phasor::phase_delta_t o = Phasor::radiansToPhase(offset);
		if (feedbackOn) {
			if (_oversampleMix < 1.0f) {
				_oversampleMix += oversampleMixIncrement;
			}
		}
		else if (_oversampleMix > 0.0f) {
			_oversampleMix -= oversampleMixIncrement;
		}

		if (_oversampleMix > 0.0f) {
			for (int i = 0; i < oversample; ++i) {
				_phasor.advancePhase();
				_buffer[i] = _sineTable.nextFromPhasor(_phasor, o);
			}
			sample = _oversampleMix * _decimator.next(_buffer);
		}
		else {
			_phasor.advancePhase(oversample);
		}
		if (_oversampleMix < 1.0f) {
			sample += (1.0f - _oversampleMix) * _sineTable.nextFromPhasor(_phasor, o);
		}

		if (_linearLevel) {
			sample *= out;
		}
		else {
			out = (1.0f - out) * Amplifier::minDecibels;
			_amplifier.setLevel(out);
			sample = _amplifier.next(sample);
		}
	}
	else {
		_phasor.advancePhase(oversample);
	}

	outputs[AUDIO_OUTPUT].value = _feedbackDelayedSample = amplitude * sample;
}

struct LinearLevelMenuItem : MenuItem {
	FMOp* _module;

	LinearLevelMenuItem(FMOp* module, const char* label)
	: _module(module)
	{
		this->text = label;
	}

	void onAction(EventAction &e) override {
		_module->_linearLevel = !_module->_linearLevel;
	}

	void step() override {
		rightText = _module->_linearLevel ? "✔" : "";
	}
};

struct FMOpWidget : ModuleWidget {
	static constexpr int hp = 10;

	FMOpWidget(FMOp* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/FMOp.svg")));
			addChild(panel);
		}

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

		// generated by svg_widgets.rb
		auto ratioParamPosition = Vec(30.0, 45.0);
		auto fineParamPosition = Vec(112.0, 57.0);
		auto attackParamPosition = Vec(107.0, 94.0);
		auto decayParamPosition = Vec(107.0, 139.0);
		auto sustainParamPosition = Vec(107.0, 184.0);
		auto releaseParamPosition = Vec(107.0, 229.0);
		auto depthParamPosition = Vec(36.0, 106.0);
		auto envToDepthParamPosition = Vec(59.0, 139.7);
		auto feedbackParamPosition = Vec(36.0, 162.0);
		auto envToFeedbackParamPosition = Vec(59.0, 195.7);
		auto levelParamPosition = Vec(36.0, 218.0);
		auto envToLevelParamPosition = Vec(59.0, 251.7);

		auto depthInputPosition = Vec(15.0, 274.0);
		auto feedbackInputPosition = Vec(47.0, 274.0);
		auto levelInputPosition = Vec(79.0, 274.0);
		auto sustainInputPosition = Vec(111.0, 274.0);
		auto pitchInputPosition = Vec(15.0, 318.0);
		auto fmInputPosition = Vec(47.0, 318.0);
		auto gateInputPosition = Vec(79.0, 318.0);

		auto audioOutputPosition = Vec(111.0, 318.0);

		auto envToDepthLightPosition = Vec(30.0, 141.0);
		auto envToFeedbackLightPosition = Vec(30.0, 197.0);
		auto envToLevelLightPosition = Vec(30.0, 253.0);
		// end generated by svg_widgets.rb

		addParam(ParamWidget::create<Knob38>(ratioParamPosition, module, FMOp::RATIO_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob16>(fineParamPosition, module, FMOp::FINE_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob26>(attackParamPosition, module, FMOp::ATTACK_PARAM, 0.0, 1.0, 0.12));
		addParam(ParamWidget::create<Knob26>(decayParamPosition, module, FMOp::DECAY_PARAM, 0.0, 1.0, 0.31623));
		addParam(ParamWidget::create<Knob26>(sustainParamPosition, module, FMOp::SUSTAIN_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<Knob26>(releaseParamPosition, module, FMOp::RELEASE_PARAM, 0.0, 1.0, 0.31623));
		addParam(ParamWidget::create<Knob26>(depthParamPosition, module, FMOp::DEPTH_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob26>(feedbackParamPosition, module, FMOp::FEEDBACK_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob26>(levelParamPosition, module, FMOp::LEVEL_PARAM, 0.0, 1.0, 1.0));
		addParam(ParamWidget::create<StatefulButton9>(envToLevelParamPosition, module, FMOp::ENV_TO_LEVEL_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<StatefulButton9>(envToFeedbackParamPosition, module, FMOp::ENV_TO_FEEDBACK_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<StatefulButton9>(envToDepthParamPosition, module, FMOp::ENV_TO_DEPTH_PARAM, 0.0, 1.0, 0.0));

		addInput(Port::create<Port24>(sustainInputPosition, Port::INPUT, module, FMOp::SUSTAIN_INPUT));
		addInput(Port::create<Port24>(depthInputPosition, Port::INPUT, module, FMOp::DEPTH_INPUT));
		addInput(Port::create<Port24>(feedbackInputPosition, Port::INPUT, module, FMOp::FEEDBACK_INPUT));
		addInput(Port::create<Port24>(levelInputPosition, Port::INPUT, module, FMOp::LEVEL_INPUT));
		addInput(Port::create<Port24>(pitchInputPosition, Port::INPUT, module, FMOp::PITCH_INPUT));
		addInput(Port::create<Port24>(gateInputPosition, Port::INPUT, module, FMOp::GATE_INPUT));
		addInput(Port::create<Port24>(fmInputPosition, Port::INPUT, module, FMOp::FM_INPUT));

		addOutput(Port::create<Port24>(audioOutputPosition, Port::OUTPUT, module, FMOp::AUDIO_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(envToLevelLightPosition, module, FMOp::ENV_TO_LEVEL_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(envToFeedbackLightPosition, module, FMOp::ENV_TO_FEEDBACK_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(envToDepthLightPosition, module, FMOp::ENV_TO_DEPTH_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
	  FMOp* fmop = dynamic_cast<FMOp*>(module);
		assert(fmop);
		menu->addChild(new MenuLabel());
		menu->addChild(new LinearLevelMenuItem(fmop, "Linear Level Response"));
	}
};

RACK_PLUGIN_MODEL_INIT(Bogaudio, FMOp) {
   Model *modelFMOp = createModel<FMOp, FMOpWidget>("Bogaudio-FMOp", "FM-OP",  "FM oscillator", OSCILLATOR_TAG, SYNTH_VOICE_TAG);
   return modelFMOp;
}
