
#include "EightFO.hpp"
#include "dsp/pitch.hpp"

void EightFO::onReset() {
	_resetTrigger.reset();
	_modulationStep = modulationSteps;
	_sampleStep = _phasor._sampleRate;
}

void EightFO::onSampleRateChange() {
	_phasor.setSampleRate(engineGetSampleRate());
	_modulationStep = modulationSteps;
	_sampleStep = _phasor._sampleRate;
}

void EightFO::step() {
	lights[SLOW_LIGHT].value = _slowMode = params[SLOW_PARAM].value > 0.5f;
	if (!(
		outputs[PHASE7_OUTPUT].active ||
		outputs[PHASE6_OUTPUT].active ||
		outputs[PHASE5_OUTPUT].active ||
		outputs[PHASE4_OUTPUT].active ||
		outputs[PHASE3_OUTPUT].active ||
		outputs[PHASE2_OUTPUT].active ||
		outputs[PHASE1_OUTPUT].active ||
		outputs[PHASE0_OUTPUT].active
	)) {
		return;
	}

	++_modulationStep;
	if (_modulationStep >= modulationSteps) {
		_modulationStep = 0;

		float frequency = params[FREQUENCY_PARAM].value;
		if (inputs[PITCH_INPUT].active) {
			frequency += inputs[PITCH_INPUT].value;
		}
		if (_slowMode) {
			frequency -= 8.0f;
		}
		else {
			frequency -= 4.0f;
		}
		frequency = cvToFrequency(frequency);
		if (frequency > 2000.0f) {
			frequency = 2000.0f;
		}
		_phasor.setFrequency(frequency);

		_wave = (Wave)int(params[WAVE_PARAM].value);
		if (_wave == SQUARE_WAVE) {
			float pw = params[SAMPLE_PWM_PARAM].value;
			if (inputs[SAMPLE_PWM_INPUT].active) {
				pw *= clamp(inputs[SAMPLE_PWM_INPUT].value / 5.0f, -1.0f, 1.0f);
			}
			pw *= 1.0f - 2.0f * _square.minPulseWidth;
			pw *= 0.5f;
			pw += 0.5f;
			_square.setPulseWidth(pw);
			_sampleSteps = 1;
		}
		else {
			float sample = fabsf(params[SAMPLE_PWM_PARAM].value);
			if (inputs[SAMPLE_PWM_INPUT].active) {
				sample *= clamp(fabsf(inputs[SAMPLE_PWM_INPUT].value) / 5.0f, 0.0f, 1.0f);
			}
			float maxSampleSteps = (_phasor._sampleRate / _phasor._frequency) / 4.0f;
			_sampleSteps = clamp((int)(sample * maxSampleSteps), 1, (int)maxSampleSteps);
			_square.setPulseWidth(SquareOscillator::defaultPulseWidth);
		}

		_offset = params[OFFSET_PARAM].value;
		if (inputs[OFFSET_INPUT].active) {
			_offset *= clamp(inputs[OFFSET_INPUT].value / 5.0f, -1.0f, 1.0f);
		}
		_offset *= 5.0f;
		_scale = params[SCALE_PARAM].value;
		if (inputs[SCALE_INPUT].active) {
			_scale *= clamp(inputs[SCALE_INPUT].value / 10.0f, 0.0f, 1.0f);
		}

		_phase7Offset = phaseOffset(params[PHASE7_PARAM], inputs[PHASE7_INPUT], basePhase7Offset);
		_phase6Offset = phaseOffset(params[PHASE6_PARAM], inputs[PHASE6_INPUT], basePhase6Offset);
		_phase5Offset = phaseOffset(params[PHASE5_PARAM], inputs[PHASE5_INPUT], basePhase5Offset);
		_phase4Offset = phaseOffset(params[PHASE4_PARAM], inputs[PHASE4_INPUT], basePhase4Offset);
		_phase3Offset = phaseOffset(params[PHASE3_PARAM], inputs[PHASE3_INPUT], basePhase3Offset);
		_phase2Offset = phaseOffset(params[PHASE2_PARAM], inputs[PHASE2_INPUT], basePhase2Offset);
		_phase1Offset = phaseOffset(params[PHASE1_PARAM], inputs[PHASE1_INPUT], basePhase1Offset);
		_phase0Offset = phaseOffset(params[PHASE0_PARAM], inputs[PHASE0_INPUT], basePhase0Offset);
	}

	if (_resetTrigger.next(inputs[RESET_INPUT].value)) {
		_phasor.resetPhase();
	}

	_phasor.advancePhase();
	bool useSample = false;
	if (_sampleSteps > 1) {
		++_sampleStep;
		if (_sampleStep >= _sampleSteps) {
			_sampleStep = 0;
		}
		else {
			useSample = true;
		}
	}
	updateOutput(useSample, outputs[PHASE7_OUTPUT], _phase7Offset, _phase7Sample, _phase7Active);
	updateOutput(useSample, outputs[PHASE6_OUTPUT], _phase6Offset, _phase6Sample, _phase6Active);
	updateOutput(useSample, outputs[PHASE5_OUTPUT], _phase5Offset, _phase5Sample, _phase5Active);
	updateOutput(useSample, outputs[PHASE4_OUTPUT], _phase4Offset, _phase4Sample, _phase4Active);
	updateOutput(useSample, outputs[PHASE3_OUTPUT], _phase3Offset, _phase3Sample, _phase3Active);
	updateOutput(useSample, outputs[PHASE2_OUTPUT], _phase2Offset, _phase2Sample, _phase2Active);
	updateOutput(useSample, outputs[PHASE1_OUTPUT], _phase1Offset, _phase1Sample, _phase1Active);
	updateOutput(useSample, outputs[PHASE0_OUTPUT], _phase0Offset, _phase0Sample, _phase0Active);
}

Phasor::phase_delta_t EightFO::phaseOffset(Param& p, Input& i, Phasor::phase_delta_t baseOffset) {
	float o = p.value * Phasor::maxPhase / 2.0f;
	if (i.active) {
		o *= clamp(i.value / 5.0f, -1.0f, 1.0f);
	}
	return baseOffset - o;
}

void EightFO::updateOutput(bool useSample, Output& output, Phasor::phase_delta_t& offset, float& sample, bool& active) {
	if (output.active) {
		if (useSample && active) {
			output.value = sample;
		}
		else {
			float v = 0.0f;
			switch (_wave) {
				case NO_WAVE: {
					assert(false);
				}
				case SINE_WAVE: {
					v = _sine.nextFromPhasor(_phasor, offset);
					break;
				}
				case TRIANGLE_WAVE: {
					v = _triangle.nextFromPhasor(_phasor, offset);
					break;
				}
				case RAMP_UP_WAVE: {
					v = _ramp.nextFromPhasor(_phasor, offset);
					break;
				}
				case RAMP_DOWN_WAVE: {
					v = -_ramp.nextFromPhasor(_phasor, offset);
					break;
				}
				case SQUARE_WAVE: {
					v = _square.nextFromPhasor(_phasor, offset);
					break;
				}
			}
			output.value = sample = amplitude * _scale * v + _offset;
		}
		active = true;
	}
	else {
		active = false;
	}
}

struct EightFOWidget : ModuleWidget {
	static constexpr int hp = 17;

	EightFOWidget(EightFO* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/EightFO.svg")));
			addChild(panel);
		}

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

		// generated by svg_widgets.rb
		auto frequencyParamPosition = Vec(40.0, 45.0);
		auto slowParamPosition = Vec(126.0, 333.7);
		auto waveParamPosition = Vec(20.0, 168.0);
		auto samplePwmParamPosition = Vec(100.0, 163.0);
		auto offsetParamPosition = Vec(40.0, 226.0);
		auto scaleParamPosition = Vec(100.0, 226.0);
		auto phase7ParamPosition = Vec(144.0, 40.0);
		auto phase6ParamPosition = Vec(144.0, 80.0);
		auto phase5ParamPosition = Vec(144.0, 120.0);
		auto phase4ParamPosition = Vec(144.0, 160.0);
		auto phase3ParamPosition = Vec(144.0, 200.0);
		auto phase2ParamPosition = Vec(144.0, 240.0);
		auto phase1ParamPosition = Vec(144.0, 280.0);
		auto phase0ParamPosition = Vec(144.0, 320.0);

		auto phase7InputPosition = Vec(179.0, 36.0);
		auto phase6InputPosition = Vec(179.0, 76.0);
		auto phase5InputPosition = Vec(179.0, 116.0);
		auto phase4InputPosition = Vec(179.0, 156.0);
		auto phase3InputPosition = Vec(179.0, 196.0);
		auto phase2InputPosition = Vec(179.0, 236.0);
		auto phase1InputPosition = Vec(179.0, 276.0);
		auto phase0InputPosition = Vec(179.0, 316.0);
		auto samplePwmInputPosition = Vec(14.0, 277.0);
		auto offsetInputPosition = Vec(52.0, 277.0);
		auto scaleInputPosition = Vec(90.0, 277.0);
		auto pitchInputPosition = Vec(14.0, 318.0);
		auto resetInputPosition = Vec(52.0, 318.0);

		auto phase7OutputPosition = Vec(218.0, 36.0);
		auto phase6OutputPosition = Vec(218.0, 76.0);
		auto phase5OutputPosition = Vec(218.0, 116.0);
		auto phase4OutputPosition = Vec(218.0, 156.0);
		auto phase3OutputPosition = Vec(218.0, 196.0);
		auto phase2OutputPosition = Vec(218.0, 236.0);
		auto phase1OutputPosition = Vec(218.0, 276.0);
		auto phase0OutputPosition = Vec(218.0, 316.0);

		auto slowLightPosition = Vec(86.0, 335.0);
		// end generated by svg_widgets.rb

		addParam(ParamWidget::create<Knob68>(frequencyParamPosition, module, EightFO::FREQUENCY_PARAM, -8.0, 5.0, 0.0));
		{
			auto w = ParamWidget::create<Knob16>(waveParamPosition, module, EightFO::WAVE_PARAM, 1.0, 5.0, 3.0);
			auto k = dynamic_cast<SVGKnob*>(w);
			k->snap = true;
			k->minAngle = 0.0;
			k->maxAngle = M_PI;
			k->speed = 3.0;
			addParam(w);
		}
		addParam(ParamWidget::create<StatefulButton9>(slowParamPosition, module, EightFO::SLOW_PARAM, 0.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob26>(samplePwmParamPosition, module, EightFO::SAMPLE_PWM_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob26>(offsetParamPosition, module, EightFO::OFFSET_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob26>(scaleParamPosition, module, EightFO::SCALE_PARAM, 0.0, 1.0, 1.0));

		addPhaseParam(phase7ParamPosition, module, EightFO::PHASE7_PARAM, Phasor::phaseToRadians(module->basePhase7Offset));
		addPhaseParam(phase6ParamPosition, module, EightFO::PHASE6_PARAM, Phasor::phaseToRadians(module->basePhase6Offset));
		addPhaseParam(phase5ParamPosition, module, EightFO::PHASE5_PARAM, Phasor::phaseToRadians(module->basePhase5Offset));
		addPhaseParam(phase4ParamPosition, module, EightFO::PHASE4_PARAM, Phasor::phaseToRadians(module->basePhase4Offset));
		addPhaseParam(phase3ParamPosition, module, EightFO::PHASE3_PARAM, Phasor::phaseToRadians(module->basePhase3Offset));
		addPhaseParam(phase2ParamPosition, module, EightFO::PHASE2_PARAM, Phasor::phaseToRadians(module->basePhase2Offset));
		addPhaseParam(phase1ParamPosition, module, EightFO::PHASE1_PARAM, Phasor::phaseToRadians(module->basePhase1Offset));
		addPhaseParam(phase0ParamPosition, module, EightFO::PHASE0_PARAM, Phasor::phaseToRadians(module->basePhase0Offset));

		addInput(Port::create<Port24>(samplePwmInputPosition, Port::INPUT, module, EightFO::SAMPLE_PWM_INPUT));
		addInput(Port::create<Port24>(offsetInputPosition, Port::INPUT, module, EightFO::OFFSET_INPUT));
		addInput(Port::create<Port24>(scaleInputPosition, Port::INPUT, module, EightFO::SCALE_INPUT));
		addInput(Port::create<Port24>(phase7InputPosition, Port::INPUT, module, EightFO::PHASE7_INPUT));
		addInput(Port::create<Port24>(phase6InputPosition, Port::INPUT, module, EightFO::PHASE6_INPUT));
		addInput(Port::create<Port24>(phase5InputPosition, Port::INPUT, module, EightFO::PHASE5_INPUT));
		addInput(Port::create<Port24>(phase4InputPosition, Port::INPUT, module, EightFO::PHASE4_INPUT));
		addInput(Port::create<Port24>(phase3InputPosition, Port::INPUT, module, EightFO::PHASE3_INPUT));
		addInput(Port::create<Port24>(phase2InputPosition, Port::INPUT, module, EightFO::PHASE2_INPUT));
		addInput(Port::create<Port24>(phase1InputPosition, Port::INPUT, module, EightFO::PHASE1_INPUT));
		addInput(Port::create<Port24>(phase0InputPosition, Port::INPUT, module, EightFO::PHASE0_INPUT));
		addInput(Port::create<Port24>(pitchInputPosition, Port::INPUT, module, EightFO::PITCH_INPUT));
		addInput(Port::create<Port24>(resetInputPosition, Port::INPUT, module, EightFO::RESET_INPUT));

		addOutput(Port::create<Port24>(phase7OutputPosition, Port::OUTPUT, module, EightFO::PHASE7_OUTPUT));
		addOutput(Port::create<Port24>(phase6OutputPosition, Port::OUTPUT, module, EightFO::PHASE6_OUTPUT));
		addOutput(Port::create<Port24>(phase5OutputPosition, Port::OUTPUT, module, EightFO::PHASE5_OUTPUT));
		addOutput(Port::create<Port24>(phase4OutputPosition, Port::OUTPUT, module, EightFO::PHASE4_OUTPUT));
		addOutput(Port::create<Port24>(phase3OutputPosition, Port::OUTPUT, module, EightFO::PHASE3_OUTPUT));
		addOutput(Port::create<Port24>(phase2OutputPosition, Port::OUTPUT, module, EightFO::PHASE2_OUTPUT));
		addOutput(Port::create<Port24>(phase1OutputPosition, Port::OUTPUT, module, EightFO::PHASE1_OUTPUT));
		addOutput(Port::create<Port24>(phase0OutputPosition, Port::OUTPUT, module, EightFO::PHASE0_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(slowLightPosition, module, EightFO::SLOW_LIGHT));
	}

	void addPhaseParam(const Vec& position, Module* module, EightFO::ParamsIds paramId, float rotation) {
		auto w = ParamWidget::create<Knob16>(position, module, paramId, -1.0, 1.0, 0.0);
		auto k = dynamic_cast<SVGKnob*>(w);
		k->minAngle += 0.5 * M_PI - rotation;
		k->maxAngle += 0.5 * M_PI - rotation;
		addParam(w);
	}
};

RACK_PLUGIN_MODEL_INIT(Bogaudio, EightFO) {
   Model *modelEightFO = createModel<EightFO, EightFOWidget>("Bogaudio-EightFO", "8FO",  "LFO with 8 phased outputs", LFO_TAG);
   return modelEightFO;
}
