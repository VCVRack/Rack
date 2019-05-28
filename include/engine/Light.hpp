#pragma once
#include <common.hpp>


namespace rack {
namespace engine {


struct Light {
	/** The square of the brightness.
	Unstable API. Use set/getBrightness().
	*/
	float value = 0.f;

	/** Sets the brightness immediately with no light decay. */
	void setBrightness(float brightness) {
		value = brightness;
	}

	float getBrightness() {
		return value;
	}

	/** Emulates light decay with slow fall but immediate rise. */
	void setSmoothBrightness(float brightness, float deltaTime) {
		if (brightness < value) {
			// Fade out light
			const float lambda = 30.f;
			value += (brightness - value) * lambda * deltaTime;
		}
		else {
			// Immediately illuminate light
			value = brightness;
		}
	}

	/** Use `setSmoothBrightness(brightness, sampleTime * frames)` instead. */
	DEPRECATED void setBrightnessSmooth(float brightness, float frames = 1.f) {
		setSmoothBrightness(brightness, frames / 44100.f);
	}
};


} // namespace engine
} // namespace rack
