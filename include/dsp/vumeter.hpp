#pragma once

#include "util/math.hpp"


namespace rack {


struct VUMeter {
	/** Decibel level difference between adjacent meter lights */
	float dBInterval = 3.0;
	float dBScaled;
	/** Value should be scaled so that 1.0 is clipping */
	void setValue(float v) {
		dBScaled = log10f(fabsf(v)) * 20.0 / dBInterval;
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
			return clamp(dBScaled + i, 0.0, 1.0);
		}
	}
};


} // namespace rack
