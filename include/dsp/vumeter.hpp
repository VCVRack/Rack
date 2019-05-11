#pragma once
#include "dsp/common.hpp"


namespace rack {
namespace dsp {


/** Deprecated. Use VuMeter2 instead. */
struct VuMeter {
	/** Decibel level difference between adjacent meter lights */
	float dBInterval = 3.0;
	float dBScaled;
	/** Value should be scaled so that 1.0 is clipping */
	void setValue(float v) {
		dBScaled = std::log10(std::fabs(v)) * 20.0 / dBInterval;
	}
	/** Returns the brightness of the light indexed by i.
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


DEPRECATED typedef VuMeter VUMeter;


/** Models a VU meter with smoothing.
Supports peak and RMS (root-mean-square) metering.
Usage example for a strip of lights with 3dB increments:
```
// Update VuMeter state every frame.
vuMeter.process(deltaTime, output);
// Iterate lights every ~512 frames (less than a screen refresh).
for (int i = 0; i < 6; i++) {
	float b = vuMeter.getBrightness(-3.f * (i + 1), -3.f * i);
	// No need to use setSmoothBrightness() since VuMeter2 smooths the value for you.
	lights[i].setBrightness(b);
}
```
*/
struct VuMeter2 {
	enum Mode {
		PEAK,
		RMS
	};
	Mode mode = PEAK;
	/** Either the smoothed peak or the mean-square of the brightness, depending on the mode. */
	float v = 0.f;
	/** Inverse time constant in 1/seconds */
	float lambda = 30.f;

	void reset() {
		v = 0.f;
	}

	void process(float deltaTime, float value) {
		if (mode == RMS) {
			value = std::pow(value, 2);
			v += (value - v) * lambda * deltaTime;
		}
		else {
			value = std::fabs(value);
			if (value >= v) {
				v = value;
			}
			else {
				v += (value - v) * lambda * deltaTime;
			}
		}
	}

	/** Returns the LED brightness measuring tick marks between dbMin and dbMax.
	For example, `getBrightness(-6.f, 0.f)` will be at minimum brightness at -6dB and maximum brightness at 0dB.
	Set dbMin == dbMax == 0.f for a clip indicator that turns fully on when db >= dbMax.
	Expensive, so call this infrequently.
	*/
	float getBrightness(float dbMin, float dbMax) {
		float db = amplitudeToDb((mode == RMS) ? std::sqrt(v) : v);
		if (db >= dbMax)
			return 1.f;
		else if (db <= dbMin)
			return 0.f;
		else
			return math::rescale(db, dbMin, dbMax, 0.f, 1.f);
	}
};


} // namespace dsp
} // namespace rack
