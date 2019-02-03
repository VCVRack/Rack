#pragma once
#include "dsp/common.hpp"


namespace rack {
namespace dsp {


/** Deprecated. */
struct VUMeter {
	/** Decibel level difference between adjacent meter lights */
	float dBInterval = 3.0;
	float dBScaled;
	/** Value should be scaled so that 1.0 is clipping */
	void setValue(float v) {
		dBScaled = std::log10(std::abs(v)) * 20.0 / dBInterval;
	}
	/** Returns the brightness of the light indexed by i
	Light 0 is a clip light (red) which is either on or off.
	All others are smooth lights which are fully bright at -dBInterval*i and higher, and fully off at -dBInterval*(i-1).
	*/
	float getBrightness(int i) {
		if (i == 0) {
			return (dBScaled >= 0.0) ? 1.0 : 0.0;
		}
		else {
			return math::clamp(dBScaled + i, 0.0, 1.0);
		}
	}
};


struct VUMeter2 {
	enum Mode {
		PEAK,
		RMS
	};
	Mode mode = PEAK;
	float v = 0.f;
	/** Inverse time constant in 1/seconds */
	float lambda = 0.f;

	void reset() {
		v = 0.f;
	}

	void process(float deltaTime, float value) {
		if (mode == PEAK) {
			value = std::abs(value);
			if (value >= v) {
				v = value;
			}
			else {
				v += (value - v) * lambda * deltaTime;
			}
		}
		else {
			value = std::pow(value, 2);
			v += (value - v) * lambda * deltaTime;
		}
	}

	/** Returns the LED brightness measuring tick marks between dbMin and dbMax.
	Set dbMin == dbMax == 0.f for a clip indicator.
	Expensive, so call this infrequently.
	*/
	float getBrightness(float dbMin, float dbMax) {
		float db = amplitudeToDb((mode == RMS) ? std::sqrt(v) : v);
		if (db > dbMax)
			return 1.f;
		else if (db < dbMin)
			return 0.f;
		else
			return math::rescale(db, dbMin, dbMax, 0.f, 1.f);
	}
};


} // namespace dsp
} // namespace rack
