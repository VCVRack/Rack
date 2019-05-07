
#include "Test.hpp"

void Test::onReset() {
}

void Test::step() {
	if (!(outputs[OUT_OUTPUT].active || outputs[OUT2_OUTPUT].active)) {
		return;
	}

#ifdef LPF
	if (!inputs[IN_INPUT].active) {
		return;
	}
	_lpf.setParams(
		engineGetSampleRate(),
		10000.0 * clamp(params[PARAM1_PARAM].value, 0.0f, 1.0f),
		std::max(10.0 * clamp(params[PARAM2_PARAM].value, 0.0f, 1.0f), 0.1)
	);
	outputs[OUT_OUTPUT].value = _lpf.next(inputs[IN_INPUT].value);

#elif LPFNOISE
	_lpf.setParams(
		engineGetSampleRate(),
		22000.0 * clamp(params[PARAM1_PARAM].value, 0.0f, 1.0f),
		0.717f
	);
	float noise = _noise.next();
	outputs[OUT_OUTPUT].value = _lpf.next(noise) * 10.0;;
	outputs[OUT2_OUTPUT].value = noise * 10.0;;

#elif SINE
	_sine.setSampleRate(engineGetSampleRate());
	_sine.setFrequency(oscillatorPitch());
	outputs[OUT_OUTPUT].value = _sine.next() * 5.0f;

	_sine2.setSampleRate(engineGetSampleRate());
	_sine2.setFrequency(oscillatorPitch());
	outputs[OUT2_OUTPUT].value = _sine2.next() * 5.0f;

#elif SQUARE
	_square.setSampleRate(engineGetSampleRate());
	_square.setFrequency(oscillatorPitch());
	float pw = params[PARAM2_PARAM].value;
	if (inputs[CV2_INPUT].active) {
		pw += clamp(inputs[CV2_INPUT].value, -5.0f, 5.0f) / 10.0f;
	}
	_square.setPulseWidth(pw);
	outputs[OUT_OUTPUT].value = _square.next() * 5.0f;

	_square2.setSampleRate(engineGetSampleRate());
	_square2.setFrequency(oscillatorPitch());
	_square2.setPulseWidth(pw);
	_square2.setQuality(params[PARAM3_PARAM].value * 200);
	outputs[OUT2_OUTPUT].value = _square2.next() * 5.0f;

#elif SAW
	_saw.setSampleRate(engineGetSampleRate());
	_saw.setFrequency(oscillatorPitch());
	outputs[OUT_OUTPUT].value = _saw.next() * 5.0f;

	_saw2.setSampleRate(engineGetSampleRate());
	_saw2.setFrequency(oscillatorPitch());
	_saw2.setQuality(params[PARAM2_PARAM].value * 200);
	outputs[OUT2_OUTPUT].value = _saw2.next() * 5.0f;

#elif SATSAW
	float saturation = params[PARAM2_PARAM].value * 10.0f;
	if (inputs[CV2_INPUT].active) {
		saturation *= clamp(inputs[CV2_INPUT].value / 10.0f, 0.0f, 1.0f);
	}
	_saw.setSampleRate(engineGetSampleRate());
	_saw.setFrequency(oscillatorPitch());
	_saw.setSaturation(saturation);
	outputs[OUT_OUTPUT].value = _saw.next() * 5.0f;

	_saw2.setSampleRate(engineGetSampleRate());
	_saw2.setFrequency(oscillatorPitch());
	_saw2.setSaturation(saturation);
	_saw2.setQuality(params[PARAM3_PARAM].value * 200);
	outputs[OUT2_OUTPUT].value = _saw2.next() * 5.0f;

#elif TRIANGLE
	_triangle.setSampleRate(engineGetSampleRate());
	_triangle.setFrequency(oscillatorPitch());
	outputs[OUT_OUTPUT].value = _triangle.next() * 5.0f;

#elif SAMPLED_TRIANGLE
	float sample = params[PARAM2_PARAM].value * Phasor::maxSampleWidth;
	if (inputs[CV2_INPUT].active) {
		sample *= clamp(inputs[CV2_INPUT].value / 10.0f, 0.0f, 1.0f);
	}
	_triangle.setSampleRate(engineGetSampleRate());
	_triangle.setFrequency(oscillatorPitch());
	_triangle.setSampleWidth(sample);
	outputs[OUT_OUTPUT].value = _triangle.next() * 5.0f;

	_triangle2.setSampleRate(engineGetSampleRate());
	_triangle2.setFrequency(oscillatorPitch());
	float maxSampleSteps = (_triangle2._sampleRate / _triangle2._frequency) / 4.0f;
	_sampleSteps = clamp((int)((4.0f * sample) * maxSampleSteps), 1, (int)maxSampleSteps);
	++_sampleStep;
	if (_sampleStep >= _sampleSteps) {
		_sampleStep = 0;
		_sample = _triangle2.next() * 5.0f;
	}
	else {
		_triangle2.advancePhase();
	}
	outputs[OUT2_OUTPUT].value = _sample;

#elif SINEBANK
	_sineBank.setSampleRate(engineGetSampleRate());
	_sineBank.setFrequency(oscillatorPitch());
	outputs[OUT_OUTPUT].value = _sineBank.next();

#elif OVERSAMPLING
	_saw1.setSampleRate(engineGetSampleRate());
	_saw1.setFrequency(oscillatorPitch() / (float)OVERSAMPLEN);
	float buf[OVERSAMPLEN];
	for (int i = 0; i < OVERSAMPLEN; ++i) {
		buf[i] = _saw1.next();
	}
	outputs[OUT_OUTPUT].value = _rackDecimator.process(buf) * 5.0f;

	_saw2.setSampleRate(engineGetSampleRate());
	_saw2.setFrequency(oscillatorPitch() / (float)OVERSAMPLEN);
	_lpf.setParams(
		engineGetSampleRate(),
		engineGetSampleRate() / 4.0f,
		0.03
	);
	_lpf2.setParams(
		engineGetSampleRate(),
		engineGetSampleRate() / 4.0f,
		0.03
	);
	float s = 0.0f;
	for (int i = 0; i < OVERSAMPLEN; ++i) {
		// s = _lpf2.next(_lpf.next(_saw2.next()));
		s = _lpf.next(_saw2.next());
		// s = _saw2.next();
	}
	outputs[OUT2_OUTPUT].value = s * 5.0;

#elif OVERSAMPLED_BL
	int quality = params[PARAM2_PARAM].value * 100;
	const int maxOversample = 16;
	int oversample = params[PARAM3_PARAM].value * maxOversample;

	_saw1.setSampleRate(engineGetSampleRate());
	_saw1.setFrequency(oscillatorPitch());
	_saw1.setQuality(quality);
	outputs[OUT_OUTPUT].value = _saw1.next() * 5.0f;

	_saw2.setSampleRate(engineGetSampleRate());
	_saw2.setQuality(quality);
	if (oversample < 2) {
		_saw2.setFrequency(oscillatorPitch());
		outputs[OUT2_OUTPUT].value = _saw2.next() * 5.0f;
	}
	else {
		_saw2.setFrequency(oscillatorPitch() / (float)oversample);
		_lpf.setParams(
			oversample * engineGetSampleRate(),
			0.95f * engineGetSampleRate(),
			0.03
		);
		float s = 0.0f;
		for (int i = 0; i < oversample; ++i) {
			s = _lpf.next(_saw2.next());
		}
		outputs[OUT2_OUTPUT].value = s * 5.0f;
	}

#elif ANTIALIASING
	const int quality = 12;
	const float oversampleThreshold = 0.06f;
	const float oversampleMixWidth = 100.0f;
	float sampleRate = engineGetSampleRate();
	float frequency = oscillatorPitch(15000.0);

	float otf = oversampleThreshold * sampleRate;
	float mix, oMix;
	if (frequency > otf) {
		if (frequency > otf + oversampleMixWidth) {
			mix = 0.0f;
			oMix = 1.0f;
		}
		else {
			oMix = (frequency - otf) / oversampleMixWidth;
			mix = 1.0f - oMix;
		}
	}
	else {
		mix = 1.0f;
		oMix = 0.0f;
	}
	assert(mix >= 0.0f);
	assert(mix <= 1.0f);
	assert(oMix >= 0.0f);
	assert(oMix <= 1.0f);

	_phasor.setSampleRate(sampleRate);
	_phasor.setFrequency(frequency);
	_oversampledPhasor.setSampleRate(sampleRate);
	_oversampledPhasor.setFrequency(frequency / (float)OVERSAMPLEN);
	_saw.setSampleRate(sampleRate);
	_saw.setQuality(quality);
	_sawDecimator.setParams(sampleRate, OVERSAMPLEN);
	_square.setSampleRate(sampleRate);
	_square.setQuality(quality);
	_squareDecimator.setParams(sampleRate, OVERSAMPLEN);

	float out = 0.0f;
	float out2 = 0.0f;
	_phasor.advancePhase();
	if (mix > 0.0f) {
		_saw.setFrequency(frequency);
		_square.setFrequency(frequency);
		out += _saw.nextFromPhasor(_phasor) * mix;
		out2 += _square.nextFromPhasor(_phasor) * mix;
	}

	if (oMix > 0.0f) {
		float sawBuf[OVERSAMPLEN] {};
		float squareBuf[OVERSAMPLEN] {};
		_saw.setFrequency(frequency / (float)OVERSAMPLEN);
		_square.setFrequency(frequency / (float)OVERSAMPLEN);

		for (int i = 0; i < OVERSAMPLEN; ++i) {
			_oversampledPhasor.advancePhase();
			sawBuf[i] = _saw.nextFromPhasor(_oversampledPhasor);
			squareBuf[i] = _square.nextFromPhasor(_oversampledPhasor);
		}

		out += _sawDecimator.next(sawBuf) * oMix;
		// out += _sawRackDecimator.process(sawBuf) * oMix;

		out2 += _squareDecimator.next(squareBuf) * oMix;
		// out2 += _squareRackDecimator.process(squareBuf) * oMix;
	}
	else {
		for (int i = 0; i < OVERSAMPLEN; ++i) {
			_oversampledPhasor.advancePhase();
		}
	}

	outputs[OUT_OUTPUT].value = out * 5.0f;
	outputs[OUT2_OUTPUT].value = out2 * 5.0f;

#elif DECIMATORS
	const int quality = 12;
	float sampleRate = engineGetSampleRate();
	float frequency = oscillatorPitch(15000.0);
	_saw.setSampleRate(sampleRate);
	_saw.setFrequency(frequency / (float)OVERSAMPLEN);
	_saw.setQuality(quality);
	_cicDecimator.setParams(sampleRate, OVERSAMPLEN);
	_lpfDecimator.setParams(sampleRate, OVERSAMPLEN);

	float buf[OVERSAMPLEN] {};
	for (int i = 0; i < OVERSAMPLEN; ++i) {
		buf[i] = _saw.next();
	}
	outputs[OUT_OUTPUT].value = _cicDecimator.next(buf) * 5.0f;
	// outputs[OUT2_OUTPUT].value = _lpfDecimator.next(buf) * 5.0f;
	outputs[OUT2_OUTPUT].value = _rackDecimator.process(buf) * 5.0f;

#elif INTERPOLATOR
	const int quality = 12;
	float sampleRate = engineGetSampleRate();
	float frequency = oscillatorPitch();
	_saw.setSampleRate(sampleRate);
	_saw.setFrequency(frequency);
	_saw.setQuality(quality);
	_decimator.setParams(sampleRate, FACTOR);
	_interpolator.setParams(sampleRate, FACTOR);

	if (_steps >= FACTOR) {
		_steps = 0;
		for (int i = 0; i < FACTOR; ++i) {
			_rawSamples[i] = _saw.next();
		}
		_interpolator.next(_decimator.next(_rawSamples), _processedSamples);
	}
	outputs[OUT_OUTPUT].value = _processedSamples[_steps] * 5.0f;
	outputs[OUT2_OUTPUT].value = _rawSamples[_steps] * 5.0f;
	++_steps;

#elif FM
	const float amplitude = 5.0f;
	float baseHz = oscillatorPitch();
	float ratio = ratio2();
	float index = index3();
	float sampleRate = engineGetSampleRate();
	if (_baseHz != baseHz || _ratio != ratio || _index != index || _sampleRate != sampleRate) {
		_baseHz = baseHz;
		_ratio = ratio;
		_index = index;
		_sampleRate = sampleRate;
		float modHz = _ratio * _baseHz;
		// printf("baseHz=%f ratio=%f modHz=%f index=%f\n", _baseHz, _ratio, modHz, _index);

		_modulator.setFrequency(modHz);
		_modulator.setSampleRate(_sampleRate);
		_carrier.setSampleRate(_sampleRate);

		_carrier2.setSampleRate(engineGetSampleRate());
		_carrier2.setFrequency(baseHz);
		_modulator2.setSampleRate(engineGetSampleRate());
		_modulator2.setFrequency(modHz);
	}

	// linear FM.
	float modHz = _ratio * _baseHz;
	_carrier.setFrequency(_baseHz + _index * _modulator.next() * modHz); // linear FM requires knowing the modulator's frequency.
	outputs[OUT_OUTPUT].value = _carrier.next() * amplitude;

	// PM for comparison - identical output.
	_carrier2.advancePhase();
	outputs[OUT2_OUTPUT].value = _carrier2.nextFromPhasor(_carrier2, Phasor::radiansToPhase(_index * _modulator2.next())) * amplitude;

#elif PM
	const float amplitude = 5.0f;
	float baseHz = oscillatorPitch();
	float modHz = ratio2() * baseHz;
	_carrier.setSampleRate(engineGetSampleRate());
	_carrier.setFrequency(baseHz);
	_modulator.setSampleRate(engineGetSampleRate());
	_modulator.setFrequency(modHz);
	_carrier.advancePhase();
	outputs[OUT_OUTPUT].value = _carrier.nextFromPhasor(_carrier, Phasor::radiansToPhase(index3() * _modulator.next())) * amplitude;

#elif FEEDBACK_PM
	_carrier.setSampleRate(engineGetSampleRate());
	_carrier.setFrequency(oscillatorPitch());
	float feedback = params[PARAM2_PARAM].value;
	if (inputs[CV2_INPUT].active) {
		feedback *= clamp(inputs[CV2_INPUT].value, 0.0f, 10.0f) / 10.0f;
	}
	_carrier.advancePhase();
	outputs[OUT_OUTPUT].value = _feedbackSample = _carrier.nextFromPhasor(_carrier, Phasor::radiansToPhase(feedback * _feedbackSample)) * 5.0f;

#elif EG
	_envelope.setSampleRate(engineGetSampleRate());
	_envelope.setAttack(params[PARAM1_PARAM].value);
	_envelope.setDecay(params[PARAM2_PARAM].value);
	_envelope.setSustain(params[PARAM3_PARAM].value);
	_envelope.setRelease(params[PARAM2_PARAM].value);
	_envelope.setGate(inputs[CV1_INPUT].value > 0.1f);
	outputs[OUT_OUTPUT].value = _envelope.next() * 10.0f;

#elif TABLES
	_sine.setSampleRate(engineGetSampleRate());
	_sine.setFrequency(oscillatorPitch());
	outputs[OUT_OUTPUT].value = _sine.next() * 5.0f;

	_table.setSampleRate(engineGetSampleRate());
	_table.setFrequency(oscillatorPitch());
	outputs[OUT2_OUTPUT].value = _table.next() * 5.0f;

#elif SLEW
	float ms = params[PARAM1_PARAM].value;
	if (inputs[CV1_INPUT].active) {
		ms *= clamp(inputs[CV2_INPUT].value, 0.0f, 10.0f) / 10.0f;
	}
	ms = powf(ms, 2.0f);
	ms *= 10000.0f;
	_slew.setParams(engineGetSampleRate(), ms);
	outputs[OUT_OUTPUT].value = _slew.next(inputs[IN_INPUT].value);

	float shape = params[PARAM2_PARAM].value;
	if (inputs[CV2_INPUT].active) {
		shape *= clamp(inputs[CV2_INPUT].value / 5.0f, -1.0f, 1.0f);
	}
	if (shape < 0.5) {
		shape /= 0.5;
		shape = _slew2.minShape + shape * (1.0f - _slew2.minShape);
	}
	else {
		shape -= 0.5f;
		shape /= 0.5f;
		shape *= (_slew2.maxShape - 1.0f);
		shape += 1.0f;
	}
	_slew2.setParams(engineGetSampleRate(), ms, shape);
	outputs[OUT2_OUTPUT].value = _slew2.next(inputs[IN_INPUT].value);

#elif RMS
	float sensitivity = params[PARAM2_PARAM].value;
	if (inputs[CV2_INPUT].active) {
		sensitivity *= clamp(inputs[CV2_INPUT].value, 0.0f, 10.0f) / 10.0f;
	}
	_rms.setSampleRate(engineGetSampleRate());
	_rms.setSensitivity(sensitivity);
	outputs[OUT_OUTPUT].value = _rms.next(inputs[IN_INPUT].value);
	_pef.setSensitivity(sensitivity);
	outputs[OUT2_OUTPUT].value = _pef.next(inputs[IN_INPUT].value);

#elif RAVG
	if (_reset.process(inputs[CV1_INPUT].value)) {
		_average.reset();
	}
	outputs[OUT_OUTPUT].value = _average.next(inputs[IN_INPUT].value);

#elif SATURATOR
	float in = inputs[IN_INPUT].value;
	outputs[OUT_OUTPUT].value = _saturator.next(in);
	outputs[OUT2_OUTPUT].value = clamp(in, -Saturator::limit, Saturator::limit);

#elif BROWNIAN
	const float maxDiv = 1000.0f;
	float change = clamp(1.0f - params[PARAM1_PARAM].value, 0.01f, 1.0f);
	float smooth = clamp(params[PARAM2_PARAM].value, 0.01f, 1.0f);
	smooth *= smooth;
	_filter1.setParams(engineGetSampleRate(), smooth * engineGetSampleRate() * 0.49f);
	_filter2.setParams(engineGetSampleRate(), smooth * engineGetSampleRate() * 0.49f);

	_last1 = _last1 + _noise1.next() / (change * maxDiv);
	outputs[OUT_OUTPUT].value = _filter1.next(_last1);
	if (_last1 > 5.0f || _last1 < -5.0f) {
		_last1 = 0.0f;
	}

	_last2 = _last2 + _noise1.next() / (change * maxDiv);
	outputs[OUT2_OUTPUT].value = _filter2.next(_last2);
	if (_last2 > 5.0f || _last2 < -5.0f) {
		_last2 = 0.0f;
	}

	// // "leaky integrator"
	// float alpha = params[PARAM1_PARAM].value;
	// alpha = clamp(1.0f - alpha*alpha, 0.00001f, 1.0f);
	// float sample = 5.0f * _noise1.next();
	// _last1 = alpha*_last1 + (1.0f - alpha)*sample;
	// outputs[OUT_OUTPUT].value = _last1;

#elif RANDOMWALK
	float change = params[PARAM1_PARAM].value;
	change *= change;
	change *= change;
	_walk1.setParams(engineGetSampleRate(), change);
	_walk2.setParams(engineGetSampleRate(), change);
	outputs[OUT_OUTPUT].value = _walk1.next();
	outputs[OUT2_OUTPUT].value = _walk2.next();
#endif
}

float Test::oscillatorPitch(float max) {
	if (inputs[CV1_INPUT].active) {
		return cvToFrequency(inputs[CV1_INPUT].value);
	}
	return max * powf(params[PARAM1_PARAM].value, 2.0);
}

float Test::oscillatorPitch2(float max) {
	if (inputs[CV2_INPUT].active) {
		return cvToFrequency(inputs[CV2_INPUT].value);
	}
	return max * powf(params[PARAM2_PARAM].value, 2.0);
}

float Test::ratio2() {
	float ratio = (params[PARAM2_PARAM].value * 2.0f) - 1.0f;
	if (inputs[CV2_INPUT].active) {
		ratio *= clamp(inputs[CV2_INPUT].value / 5.0f, -1.0f, 1.0f);
	}
	if (ratio < 0.0f) {
		return 1.0f + ratio;
	}
	return 1.0f + 9.0f*ratio;
}

float Test::index3() {
	float index = params[PARAM3_PARAM].value;
	if (inputs[CV3_INPUT].active) {
		index *= clamp(inputs[CV3_INPUT].value, 0.0f, 10.0f) / 10.0f;
	}
	return index * 10.0f;
}


struct TestWidget : ModuleWidget {
	TestWidget(Test* module) : ModuleWidget(module) {
		box.size = Vec(RACK_GRID_WIDTH * 3, RACK_GRID_HEIGHT);

		{
			SVGPanel *panel = new SVGPanel();
			panel->box.size = box.size;
			panel->setBackground(SVG::load(assetPlugin(plugin, "res/Test.svg")));
			addChild(panel);
		}

		addChild(Widget::create<ScrewSilver>(Vec(0, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto param1ParamPosition = Vec(9.5, 13.5);
		auto param2ParamPosition = Vec(9.5, 98.5);
		auto param3ParamPosition = Vec(9.5, 183.5);

		auto cv1InputPosition = Vec(10.5, 53.0);
		auto cv2InputPosition = Vec(10.5, 138.0);
		auto cv3InputPosition = Vec(10.5, 223.0);
		auto inInputPosition = Vec(10.5, 268.0);

		auto outOutputPosition = Vec(10.5, 306.0);
		auto out2OutputPosition = Vec(20.5, 316.0);
		// end generated by svg_widgets.rb

		addParam(ParamWidget::create<Knob26>(param1ParamPosition, module, Test::PARAM1_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Knob26>(param2ParamPosition, module, Test::PARAM2_PARAM, 0.0, 1.0, 0.5));
		addParam(ParamWidget::create<Knob26>(param3ParamPosition, module, Test::PARAM3_PARAM, 0.0, 1.0, 0.5));

		addInput(Port::create<Port24>(cv1InputPosition, Port::INPUT, module, Test::CV1_INPUT));
		addInput(Port::create<Port24>(cv2InputPosition, Port::INPUT, module, Test::CV2_INPUT));
		addInput(Port::create<Port24>(cv3InputPosition, Port::INPUT, module, Test::CV3_INPUT));
		addInput(Port::create<Port24>(inInputPosition, Port::INPUT, module, Test::IN_INPUT));

		addOutput(Port::create<Port24>(outOutputPosition, Port::OUTPUT, module, Test::OUT_OUTPUT));
		addOutput(Port::create<Port24>(out2OutputPosition, Port::OUTPUT, module, Test::OUT2_OUTPUT));
	}
};

Model* modelTest = Model::create<Test, TestWidget>("Bogaudio", "Bogaudio-Test", "Test");
