
#include "Additator.hpp"

void Additator::onReset() {
	_syncTrigger.reset();
	_steps = modulationSteps;
	_phase = PHASE_RESET;
}

void Additator::onSampleRateChange() {
	float sampleRate = engineGetSampleRate();
	_oscillator.setSampleRate(sampleRate);
	_maxFrequency = 0.475f * sampleRate;
	_steps = modulationSteps;
	_phase = PHASE_RESET;
	_widthSL.setParams(sampleRate, slewLimitTime, maxWidth);
	_oddSkewSL.setParams(sampleRate, slewLimitTime, 2.0f * maxSkew);
	_evenSkewSL.setParams(sampleRate, slewLimitTime, 2.0f * maxSkew);
	_amplitudeNormalizationSL.setParams(sampleRate, slewLimitTime, maxAmplitudeNormalization - minAmplitudeNormalization);
	_decaySL.setParams(sampleRate, slewLimitTime, maxDecay - minDecay);
	_balanceSL.setParams(sampleRate, slewLimitTime, 2.0f);
	_filterSL.setParams(sampleRate, slewLimitTime, maxFilter - minFilter);
}

float Additator::cvValue(Input& cv, bool dc) {
	if (!cv.active) {
		return dc ? 1.0f : 0.0f;
	}
	if (dc) {
		return clamp(cv.value / 10.0f, 0.0f, 1.0f);
	}
	return clamp(cv.value / 5.0f, -1.0f, 1.0f);
}

void Additator::step() {
	if (!outputs[AUDIO_OUTPUT].active) {
		Phase phase = params[PHASE_PARAM].value > 1.5f ? PHASE_COSINE : PHASE_SINE;
		lights[SINE_LIGHT].value = phase == PHASE_SINE;
		lights[COSINE_LIGHT].value = phase == PHASE_COSINE;
		return;
	}
	lights[SINE_LIGHT].value = _phase == PHASE_SINE;
	lights[COSINE_LIGHT].value = _phase == PHASE_COSINE;

	++_steps;
	if (_steps >= modulationSteps) {
		_steps = 0;

		float width = _widthSL.next(clamp(params[WIDTH_PARAM].value + (maxWidth / 2.0f) * cvValue(inputs[WIDTH_INPUT]), 0.0f, maxWidth));
		float oddSkew = _oddSkewSL.next(clamp(params[ODD_SKEW_PARAM].value + cvValue(inputs[ODD_SKEW_INPUT]), -maxSkew, maxSkew));
		float evenSkew = _evenSkewSL.next(clamp(params[EVEN_SKEW_PARAM].value + cvValue(inputs[EVEN_SKEW_INPUT]), -maxSkew, maxSkew));
		if (
			_width != width ||
			_oddSkew != oddSkew ||
			_evenSkew != evenSkew
		) {
			_width = width;
			_oddSkew = oddSkew;
			_evenSkew = evenSkew;

			float multiple = 1.0f;
			_oscillator.setPartialFrequencyRatio(1, multiple);
			_activePartials = 1;
			for (int i = 2, n = _oscillator.partialCount(); i <= n; ++i) {
				float ii = i;
				if (i % 2 == 0) {
					ii += _evenSkew;
				}
				else {
					ii += _oddSkew;
				}
				if (_oscillator.setPartialFrequencyRatio(i, powf(ii, _width))) {
					_activePartials = i;
				}
			}
		}

		int partials = clamp((int)roundf(params[PARTIALS_PARAM].value * cvValue(inputs[PARTIALS_INPUT], true)), 0, maxPartials);
		float amplitudeNormalization = _amplitudeNormalizationSL.next(clamp(params[GAIN_PARAM].value + ((maxAmplitudeNormalization - minAmplitudeNormalization) / 2.0f) * cvValue(inputs[GAIN_INPUT]), minAmplitudeNormalization, maxAmplitudeNormalization));
		float decay = _decaySL.next(clamp(params[DECAY_PARAM].value + ((maxDecay - minDecay) / 2.0f) * cvValue(inputs[DECAY_INPUT]), minDecay, maxDecay));
		float balance = _balanceSL.next(clamp(params[BALANCE_PARAM].value + cvValue(inputs[BALANCE_INPUT]), -1.0f, 1.0f));
		float filter = _filterSL.next(clamp(params[FILTER_PARAM].value + cvValue(inputs[FILTER_INPUT]), minFilter, maxFilter));
		if (
			_partials != partials ||
			_amplitudeNormalization != amplitudeNormalization ||
			_decay != decay ||
			_balance != balance ||
			_filter != filter
		) {
			int envelopes = _partials != partials ? std::max(_partials, partials) : 0;
			_partials = partials;
			_amplitudeNormalization = amplitudeNormalization;
			_decay = decay;
			_balance = balance;
			_filter = filter;

#ifdef _MSC_VER
			float *as = new float[maxPartials + 1];
#else
			float as[maxPartials + 1];
#endif
			float total = as[1] = 1.0f;
			filter = log10f(_filter) + 1.0f;
			int np = std::min(_partials, _activePartials);
			for (int i = 2, n = _oscillator.partialCount(); i <= n; ++i) {
				as[i] = 0.0f;
				if (i <= np) {
					as[i] = powf(i, -_decay) * powf(_filter, i);
					if (i % 2 == 0) {
						if (_balance > 0.0f) {
							as[i] *= 1.0f - _balance;
						}
					}
					else {
						if (_balance < 0.0f) {
							as[i] *= 1.0f + _balance;
						}
					}
					total += as[i];
				}
			}
			float norm = std::max(np / (float)_oscillator.partialCount(), 0.1f);
			norm = 1.0f + (_amplitudeNormalization - 1.0f) * norm;
			norm = std::max(total / norm, 0.7f);
			for (int i = 1, n = _oscillator.partialCount(); i <= n; ++i) {
				as[i] /= norm;
				_oscillator.setPartialAmplitude(i, as[i], i <= envelopes);
			}
#ifdef _MSC_VER
         delete [] as;
#endif
		}

		float frequency = params[FREQUENCY_PARAM].value;
		frequency += params[FINE_PARAM].value / 12.0f;;
		if (inputs[PITCH_INPUT].active) {
			frequency += clamp(inputs[PITCH_INPUT].value, -5.0f, 5.0f);
		}
		frequency = clamp(cvToFrequency(frequency), 20.0f, _maxFrequency);
		_oscillator.setFrequency(frequency);

		Phase phase = params[PHASE_PARAM].value > 1.5f ? PHASE_COSINE : PHASE_SINE;
		if (_phase != phase) {
			_phase = phase;
			_oscillator.syncToPhase(_phase == PHASE_SINE ? 0.0f : M_PI / 2.0f);
		}
	}

	if (_syncTrigger.next(inputs[SYNC_INPUT].value)) {
		_oscillator.syncToPhase(_phase == PHASE_SINE ? 0.0f : M_PI / 2.0f);
	}
	outputs[AUDIO_OUTPUT].value = _oscillator.next() * 5.0;
}

struct AdditatorWidget : ModuleWidget {
	static constexpr int hp = 15;

	AdditatorWidget(Additator* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Additator.svg")));
			addChild(panel);
		}

		addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

		// generated by svg_widgets.rb
		auto frequencyParamPosition = Vec(40.0, 45.0);
		auto partialsParamPosition = Vec(165.0, 60.0);
		auto fineParamPosition = Vec(30.0, 160.0);
		auto widthParamPosition = Vec(79.0, 155.0);
		auto oddSkewParamPosition = Vec(132.0, 155.0);
		auto evenSkewParamPosition = Vec(184.0, 155.0);
		auto gainParamPosition = Vec(25.0, 218.0);
		auto decayParamPosition = Vec(79.0, 218.0);
		auto balanceParamPosition = Vec(132.0, 218.0);
		auto filterParamPosition = Vec(184.0, 218.0);
		auto phaseParamPosition = Vec(194.0, 299.0);

		auto syncInputPosition = Vec(16.0, 274.0);
		auto partialsInputPosition = Vec(50.0, 274.0);
		auto widthInputPosition = Vec(84.0, 274.0);
		auto oddSkewInputPosition = Vec(118.0, 274.0);
		auto evenSkewInputPosition = Vec(152.0, 274.0);
		auto pitchInputPosition = Vec(16.0, 318.0);
		auto gainInputPosition = Vec(50.0, 318.0);
		auto decayInputPosition = Vec(84.0, 318.0);
		auto balanceInputPosition = Vec(118.0, 318.0);
		auto filterInputPosition = Vec(152.0, 318.0);

		auto audioOutputPosition = Vec(186.0, 318.0);

		auto sineLightPosition = Vec(185.0, 272.0);
		auto cosineLightPosition = Vec(185.0, 287.0);
		// end generated by svg_widgets.rb

		addParam(ParamWidget::create<Knob68>(frequencyParamPosition, module, Additator::FREQUENCY_PARAM, -3.0, 6.0, 0.0));
		{
			auto w = ParamWidget::create<Knob38>(partialsParamPosition, module, Additator::PARTIALS_PARAM, 1.0, module->maxPartials, module->maxPartials / 5.0);
			dynamic_cast<Knob*>(w)->snap = true;
			addParam(w);
		}
		addParam(ParamWidget::create<Knob16>(fineParamPosition, module, Additator::FINE_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob26>(widthParamPosition, module, Additator::WIDTH_PARAM, 0.0, module->maxWidth, module->maxWidth / 2.0));
		addParam(ParamWidget::create<Knob26>(oddSkewParamPosition, module, Additator::ODD_SKEW_PARAM, -module->maxSkew, module->maxSkew, 0.0));
		addParam(ParamWidget::create<Knob26>(evenSkewParamPosition, module, Additator::EVEN_SKEW_PARAM, -module->maxSkew, module->maxSkew, 0.0));
		addParam(ParamWidget::create<Knob26>(
			gainParamPosition,
			module,
			Additator::GAIN_PARAM,
			module->minAmplitudeNormalization,
			module->maxAmplitudeNormalization,
			(module->maxAmplitudeNormalization - module->minAmplitudeNormalization) / 2.0 + module->minAmplitudeNormalization
		));
		addParam(ParamWidget::create<Knob26>(
			decayParamPosition,
			module,
			Additator::DECAY_PARAM,
			module->minDecay,
			module->maxDecay,
			(module->maxDecay - module->minDecay) / 2.0 + module->minDecay
		));
		addParam(ParamWidget::create<Knob26>(balanceParamPosition, module, Additator::BALANCE_PARAM, -1.0, 1.0, 0.0));
		addParam(ParamWidget::create<Knob26>(
			filterParamPosition,
			module,
			Additator::FILTER_PARAM,
			module->minFilter,
			module->maxFilter,
			(module->maxFilter - module->minFilter) / 2.0 + module->minFilter
		));
		addParam(ParamWidget::create<StatefulButton9>(phaseParamPosition, module, Additator::PHASE_PARAM, 1.0, 2.0, 1.0));

		addInput(Port::create<Port24>(partialsInputPosition, Port::INPUT, module, Additator::PARTIALS_INPUT));
		addInput(Port::create<Port24>(widthInputPosition, Port::INPUT, module, Additator::WIDTH_INPUT));
		addInput(Port::create<Port24>(oddSkewInputPosition, Port::INPUT, module, Additator::ODD_SKEW_INPUT));
		addInput(Port::create<Port24>(evenSkewInputPosition, Port::INPUT, module, Additator::EVEN_SKEW_INPUT));
		addInput(Port::create<Port24>(gainInputPosition, Port::INPUT, module, Additator::GAIN_INPUT));
		addInput(Port::create<Port24>(decayInputPosition, Port::INPUT, module, Additator::DECAY_INPUT));
		addInput(Port::create<Port24>(balanceInputPosition, Port::INPUT, module, Additator::BALANCE_INPUT));
		addInput(Port::create<Port24>(filterInputPosition, Port::INPUT, module, Additator::FILTER_INPUT));
		addInput(Port::create<Port24>(pitchInputPosition, Port::INPUT, module, Additator::PITCH_INPUT));
		addInput(Port::create<Port24>(syncInputPosition, Port::INPUT, module, Additator::SYNC_INPUT));

		addOutput(Port::create<Port24>(audioOutputPosition, Port::OUTPUT, module, Additator::AUDIO_OUTPUT));

		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(sineLightPosition, module, Additator::SINE_LIGHT));
		addChild(ModuleLightWidget::create<SmallLight<GreenLight>>(cosineLightPosition, module, Additator::COSINE_LIGHT));
	}
};

RACK_PLUGIN_MODEL_INIT(Bogaudio, Additator) {
   Model *modelAdditator = createModel<Additator, AdditatorWidget>("Bogaudio-Additator", "Additator",  "additive oscillator", OSCILLATOR_TAG);
   return modelAdditator;
}
