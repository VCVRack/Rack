
#include <math.h>
#include <assert.h>

#include "filter.hpp"

using namespace bogaudio::dsp;

void ComplexBiquadFilter::setComplexParams(
	float gain,
	float zeroRadius,
	float zeroTheta,
	float poleRadius,
	float poleTheta
) {
	if (
		_gain != gain ||
		_zeroRadius != zeroRadius ||
		_zeroTheta != zeroTheta ||
		_poleRadius != poleRadius ||
		_poleTheta != poleTheta
	) {
		assert(gain >= 0.0f && gain <= 1.0f);
		assert(zeroRadius >= 0.0f && zeroRadius <= 1.0f);
		assert(zeroTheta >= 0.0f && zeroTheta <= 2.0f*M_PI);
		assert(poleRadius >= 0.0f && poleRadius <= 1.0f);
		assert(poleTheta >= 0.0f && poleTheta <= 2.0f*M_PI);
		_gain = gain;
		_zeroRadius = zeroRadius;
		_zeroTheta = zeroTheta;
		_poleRadius = poleRadius;
		_poleTheta = poleTheta;
		updateParams();
	}
}

void ComplexBiquadFilter::updateParams() {
	setParams(
		_gain,
		-2.0f * _zeroRadius * cosf(_zeroTheta) * _gain,
		_zeroRadius * _zeroRadius * _gain,
		1.0f,
		-2.0f * _poleRadius * cosf(_poleTheta),
		_poleRadius * _poleRadius
	);
}


// See: http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
void LowPassFilter::setParams(float sampleRate, float cutoff, float q) {
	if (_sampleRate == sampleRate && _cutoff == cutoff && _q == q) {
		return;
	}
	_sampleRate = sampleRate;
	_cutoff = cutoff;
	_q = q;
	// printf("\nLPF set param: sr=%f c=%f q=%f\n", _sampleRate, _cutoff, _q);

	float w0 = 2.0 * M_PI * _cutoff / _sampleRate;
	float cosw0 = cosf(w0);
	float alpha = sinf(w0) / (2.0 * _q);

	_biquad.setParams(
		(1.0 - cosw0) / 2.0,
		1.0 - cosw0,
		(1.0 - cosw0) / 2.0,
		1.0 + alpha,
		-2.0 * cosw0,
		1.0 - alpha
	);
}


// Adapted from Smith 1997, "The Scientist and Engineer's Guide to DSP"
void MultipoleFilter::setParams(
	Type type,
	int poles,
	float sampleRate,
	float cutoff,
	float ripple
) {
	if (
		_type == type &&
		_poles == poles &&
		_sampleRate == sampleRate &&
		_cutoff == cutoff &&
		_ripple == ripple
	) {
		return;
	}
	assert(poles >= 1 && poles <= maxPoles);
	assert(poles % 2 == 0); // relax this later?
	assert(sampleRate >= 0.0f);
	assert(cutoff >= 0.0f && cutoff <= sampleRate / 2.0f);
	assert(ripple >= 0.0f && ripple <= maxRipple);
	_type = type;
	_poles = poles;
	_sampleRate = sampleRate;
	_cutoff = cutoff;
	_ripple = ripple;

	for (int p = 0, pn = _poles / 2; p < pn; ++p) {
		double a0 = 0.0;
		double a1 = 0.0;
		double a2 = 0.0;
		double b1 = 0.0;
		double b2 = 0.0;
		{
			double angle = M_PI/(_poles*2.0) + p*M_PI/_poles;
			double rp = -cos(angle);
			double ip = sin(angle);

			if (_ripple > 0.01f) {
				double es = sqrt(pow(1.0 / (1.0 - _ripple), 2.0) - 1.0);
				double esi = 1.0 / es;
				double esis = esi * esi;
				double polesi = 1.0 / (float)_poles;
				double vx = polesi * log(esi + sqrt(esis + 1.0));
				double kx = polesi * log(esi + sqrt(esis - 1.0));
				kx = (exp(kx) + exp(-kx)) / 2.0;
				rp *= ((exp(vx) - exp(-vx)) / 2.0) / kx;
				ip *= ((exp(vx) + exp(-vx)) / 2.0) / kx;
				// printf("\n\n\ncheb p=%d pn=%d rip=%f rp=%f ip=%f es=%f vx=%f kx=%f\n", p, pn, _ripple, rp, ip, es, vx, kx);
			}

			const double t = 2.0 * tan(0.5);
			const double ts = t * t;
			// printf("\n\nco=%f sr=%f fc=%f\n", _cutoff, _sampleRate, _cutoff / _sampleRate);
			double m = rp*rp + ip*ip;
			double mts = m * ts;
			double d = 4.0 - 4.0*rp*t + mts;
			double x0 = ts / d;
			double x1 = 2.0 * x0;
			double x2 = x0;
			double y1 = (8.0 - 2.0*mts) / d;
			double y2 = (-4.0 - 4.0*rp*t - mts) / d;
			// printf("vs p=%d t=%f w=%f m=%f d=%f x0=%f x1=%f x2=%f y1=%f y2=%f\n", p, t, w, m, d, x0, x1, x2, y1, y2);

			// FIXME: optimization: everything above here only depends on type, poles and ripple.

			double w = 2.0 * M_PI * (_cutoff / _sampleRate);
			double w2 = w / 2.0;
			double k = 0.0;
			switch (_type) {
				case LP_TYPE: {
					k = sin(0.5 - w2) / sin(0.5 + w2);
					break;
				}
				case HP_TYPE: {
					k = -cos(w2 + 0.5) / cos(w2 - 0.5);
					break;
				}
			}
			double ks = k * k;
			d = 1.0 + y1*k - y2*ks;
			a0 = (x0 - x1*k + x2*ks) / d;
			a1 = (-2.0*x0*k + x1 + x1*ks - 2.0*x2*k) / d;
			a2 = (x0*ks - x1*k + x2) / d;
			b1 = (2.0*k + y1 + y1*ks - 2.0*y2*k) / d;
			b2 = (-ks - y1*k + y2) / d;
			if (_type == HP_TYPE) {
				a1 = -a1;
				b1 = -b1;
			}

			// printf("pole %d: rp=%f ip=%f a0=%f a1=%f a2=%f b1=%f b2=%f\n\n\n", p, rp, ip, a0, a1, a2, b1, b2);
			_biquads[p].setParams(a0, a1, a2, 1.0, -b1, -b2);
		}
	}
}

float MultipoleFilter::next(float sample) {
	for (int p = 0, pn = _poles / 2; p < pn; ++p) {
		sample = _biquads[p].next(sample);
	}
	return sample;
}


void LPFDecimator::setParams(float sampleRate, int factor) {
	_factor = factor;
	_filter.setParams(
		MultipoleFilter::LP_TYPE,
		4,
		factor * sampleRate,
		0.45f * sampleRate,
		0
	);
}

float LPFDecimator::next(const float* buf) {
	float s = 0.0f;
	for (int i = 0; i < _factor; ++i) {
		s = _filter.next(buf[i]);
	}
	return s;
}


// cascaded integrator–comb decimator: https://en.wikipedia.org/wiki/Cascaded_integrator%E2%80%93comb_filter
CICDecimator::CICDecimator(int stages, int factor) {
	assert(stages > 0);
	_stages = stages;
	_integrators = new T[_stages + 1] {};
	_combs = new T[_stages] {};
	setParams(0.0f, factor);
}

CICDecimator::~CICDecimator() {
	delete[] _integrators;
	delete[] _combs;
}

void CICDecimator::setParams(float _sampleRate, int factor) {
	assert(factor > 0);
	if (_factor != factor) {
		_factor = factor;
		_gainCorrection = 1.0f / (float)(pow(_factor, _stages));
	}
}

float CICDecimator::next(const float* buf) {
	for (int i = 0; i < _factor; ++i) {
		_integrators[0] = buf[i] * scale;
		for (int j = 1; j <= _stages; ++j) {
			_integrators[j] += _integrators[j - 1];
		}
	}
	T s = _integrators[_stages];
	for (int i = 0; i < _stages; ++i) {
		T t = s;
		s -= _combs[i];
		_combs[i] = t;
	}
	return _gainCorrection * (s / (float)scale);
}
