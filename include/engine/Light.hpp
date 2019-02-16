#pragma once
#include "common.hpp"


namespace rack {
namespace engine {


struct Light {
	/** The square of the brightness.
	Unstable API. Use set/getBrightness().
	*/
	float value = 0.f;

	/** Sets the brightness immediately with no light decay. */
	void setBrightness(float brightness) {
		value = (brightness > 0.f) ? std::pow(brightness, 2) : 0.f;
	}

	float getBrightness() {
		return std::sqrt(value);
	}

	/** Emulates light decay with slow fall but immediate rise. */
	void setSmoothBrightness(float brightness, float deltaTime) {
		float v = (brightness > 0.f) ? std::pow(brightness, 2) : 0.f;
		if (v < value) {
			// Fade out light
			const float lambda = 30.f;
			value += (v - value) * lambda * deltaTime;
		}
		else {
			// Immediately illuminate light
			value = v;
		}
	}

	/** Use `setSmoothBrightness(brightness, APP->engine->getSampleTime())` instead. */
	DEPRECATED void setBrightnessSmooth(float brightness, float frames = 1.f) {
		setSmoothBrightness(brightness, frames / 44100.f);
	}
};


} // namespace engine
} // namespace rack
