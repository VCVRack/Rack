
#include "oscillator.hpp"

using namespace bogaudio::dsp;

void Phasor::setSampleWidth(float sw) {
	if (sw < 0.0f) {
		sw = 0.0f;
	}
	else if (sw > maxSampleWidth) {
		sw = maxSampleWidth;
	}
	if (_sampleWidth != sw) {
		_sampleWidth = sw;
		if (_sampleWidth > 0.001f) {
			_samplePhase = _sampleWidth * (float)maxPhase;
		}
		else {
			_samplePhase = 0;
		}
	}
}

void Phasor::resetPhase() {
	_phase = 0;
}

void Phasor::setPhase(float radians) {
	_phase = radiansToPhase(radians);
}

float Phasor::nextFromPhasor(const Phasor& phasor, phase_delta_t offset) {
	offset += phasor._phase;
	if (_samplePhase > 0) {
		offset -= offset % _samplePhase;
	}
	return _nextForPhase(offset);
}

void Phasor::_update() {
	_delta = ((phase_delta_t)((_frequency / _sampleRate) * maxPhase)) % maxPhase;
}

float Phasor::_next() {
	advancePhase();
	if (_samplePhase > 0) {
		return _nextForPhase(_phase - (_phase % _samplePhase));
	}
	return _nextForPhase(_phase);
}

float Phasor::_nextForPhase(phase_t phase) {
	return phase;
}


float TablePhasor::_nextForPhase(phase_t phase) {
	if (_tableLength >= 1024) {
		int i = (((((uint64_t)phase) << 16) / maxPhase) * _tableLength) >> 16;
		if (i >= _tableLength) {
			i %= _tableLength;
		}
		return _table.value(i);
	}

	float fi = (phase / (float)maxPhase) * _tableLength;
	int i = (int)fi;
	if (i >= _tableLength) {
		i %= _tableLength;
	}
	float v1 = _table.value(i);
	float v2 = _table.value(i + 1 == _tableLength ? 0 : i + 1);
	return v1 + (fi - i)*(v2 - v1);
}


// A New Recursive Quadrature Oscillator, Martin Vicanek, 2015 - http://vicanek.de/articles/QuadOsc.pdf
void SineOscillator::setPhase(double phase) {
	_x = cos(phase);
	_y = sin(phase);
}

void SineOscillator::update() {
	double w = (_frequency / _sampleRate) * 2.0 * M_PI;
	_k1 = tan(w / 2.0);
	_k2 = 2.0 * _k1 / (1 + _k1*_k1);
}

float SineOscillator::_next() {
	double t = _x - _k1*_y;
	_y = _y + _k2*t;
	_x = t - _k1*_y;
	return _y;
}


float SawOscillator::_nextForPhase(phase_t phase) {
	return (phase / (float)maxPhase) * 2.0f - 1.0f;
}


void SaturatingSawOscillator::setSaturation(float saturation) {
	if (_saturation != saturation) {
#if 0
		assert(saturation >= 0.0f);
#endif
		_saturation = saturation;
		if (_saturation >= 0.1f) {
			if (_saturation < 1.0f) {
				_saturationNormalization = 1.0f / tanhf(_saturation * M_PI);
			}
			else {
				_saturationNormalization = 1.0f;
			}
		}
	}
}

float SaturatingSawOscillator::_nextForPhase(phase_t phase) {
	float sample = SawOscillator::_nextForPhase(phase);
	if (_saturation >= 0.1f) {
		sample = _tanhf.value(sample * _saturation * M_PI) * _saturationNormalization;
	}
	return sample;
}


void BandLimitedSawOscillator::setQuality(int quality) {
	if (_quality != quality) {
#if 0
		assert(quality >= 0);
#endif
		_quality = quality;
		_update();
	}
}

void BandLimitedSawOscillator::_update() {
	Phasor::_update();
#ifdef _MSC_VER
#define sMIN(a,b) (((a)>(b))?(b):(a))
	int q = sMIN(_quality, (int)(0.5f * (_sampleRate / _frequency)));
#else
	int q = std::min(_quality, (int)(0.5f * (_sampleRate / _frequency)));
#endif
	_qd = q * _delta;
}

float BandLimitedSawOscillator::_nextForPhase(phase_t phase) {
	float sample = SaturatingSawOscillator::_nextForPhase(phase);
	if (phase > maxPhase - _qd) {
		float i = (maxPhase - phase) / (float)_qd;
		i = (1.0f - i) * _halfTableLen;
		sample -= _table.value((int)i);
	}
	else if (phase < _qd) {
		float i = phase / (float)_qd;
		i *= _halfTableLen - 1;
		i += _halfTableLen;
		sample -= _table.value((int)i);
	}
	return sample;
}


void SquareOscillator::setPulseWidth(float pw) {
	if (_pulseWidthInput == pw) {
		return;
	}
	_pulseWidthInput = pw;

	if (pw >= maxPulseWidth) {
		pw = maxPulseWidth;
	}
	else if (pw <= minPulseWidth) {
		pw = minPulseWidth;
	}
	_pulseWidth = maxPhase * pw;
}

float SquareOscillator::_nextForPhase(phase_t phase) {
	if (positive) {
		if (phase >= _pulseWidth) {
			positive = false;
			return -1.0f;
		}
		return 1.0f;
	}
	if (phase < _pulseWidth) {
		positive = true;
		return 1.0f;
	}
	return -1.0f;
}


void BandLimitedSquareOscillator::setPulseWidth(float pw) {
	if (_pulseWidthInput == pw) {
		return;
	}
	_pulseWidthInput = pw;

	if (pw >= maxPulseWidth) {
		pw = maxPulseWidth;
	}
	else if (pw <= minPulseWidth) {
		pw = minPulseWidth;
	}
	_pulseWidth = maxPhase * pw;

	if (pw > 0.5) {
		_offset = 2.0f * pw - 1.0f;
	}
	else {
		_offset = -(1.0f - 2.0f * pw);
	}
}

float BandLimitedSquareOscillator::_nextForPhase(phase_t phase) {
	float sample = -BandLimitedSawOscillator::_nextForPhase(phase);
	sample += BandLimitedSawOscillator::_nextForPhase(phase - _pulseWidth);
	return sample + _offset;
}


float TriangleOscillator::_nextForPhase(phase_t phase) {
	float p = (phase / (float)maxPhase) * 4.0f;
	if (phase < quarterMaxPhase) {
		return p;
	}
	if (phase < threeQuartersMaxPhase) {
		return 2.0f - p;
	}
	return p - 4.0f;
}


void SineBankOscillator::setPartial(int i, float frequencyRatio, float amplitude) {
	setPartialFrequencyRatio(i, frequencyRatio);
	setPartialAmplitude(i, amplitude);
}

bool SineBankOscillator::setPartialFrequencyRatio(int i, float frequencyRatio) {
	if (i <= (int)_partials.size()) {
		Partial& p = _partials[i - 1];
		p.frequencyRatio = frequencyRatio;
		double f = (double)_frequency * (double)frequencyRatio;
		p.frequency = f;
		p.sine.setFrequency(f);
		return f < _maxPartialFrequency;
	}
	return false;
}

void SineBankOscillator::setPartialAmplitude(int i, float amplitude, bool envelope) {
	if (i <= (int)_partials.size()) {
		Partial& p = _partials[i - 1];
		if (envelope) {
			p.amplitudeTarget = amplitude;
			p.amplitudeStepDelta = (amplitude - p.amplitude) / (float)_amplitudeEnvelopeSamples;
			p.amplitudeSteps = _amplitudeEnvelopeSamples;
		}
		else if (p.amplitudeSteps > 0) {
			p.amplitudeTarget = amplitude;
			p.amplitudeStepDelta = (amplitude - p.amplitude) / (float)p.amplitudeSteps;
		}
		else {
			p.amplitude = amplitude;
		}
	}
}

void SineBankOscillator::syncToPhase(float phase) {
	for (Partial& p : _partials) {
		p.sine.setPhase(phase);
	}
}

void SineBankOscillator::_sampleRateChanged() {
	_maxPartialFrequency = _maxPartialFrequencySRRatio * _sampleRate;
	_amplitudeEnvelopeSamples = _sampleRate * (_amplitudeEnvelopeMS / 1000.0f);
	for (Partial& p : _partials) {
		p.sine.setSampleRate(_sampleRate);
	}
}

void SineBankOscillator::_frequencyChanged() {
	for (Partial& p : _partials) {
		p.frequency = _frequency * p.frequencyRatio;
		p.sine.setFrequency(_frequency * p.frequencyRatio);
	}
}

float SineBankOscillator::next(Phasor::phase_t phaseOffset) {
	float next = 0.0;
	for (Partial& p : _partials) {
		p.sine.advancePhase();
		if (p.frequency < _maxPartialFrequency && (p.amplitude > 0.001 || p.amplitude < -0.001 || p.amplitudeSteps > 0)) {
			if (p.amplitudeSteps > 0) {
				if (p.amplitudeSteps == 1) {
					p.amplitude = p.amplitudeTarget;
				}
				else {
					p.amplitude += p.amplitudeStepDelta;
				}
				--p.amplitudeSteps;
			}
			next += p.sine.nextFromPhasor(p.sine, phaseOffset) * p.amplitude;
		}
	}
	return next;
}
