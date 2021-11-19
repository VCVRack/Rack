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

	/** Emulates light decay with slow fall but immediate rise.
	Default lambda set to roughly 2 screen frames.
	*/
	void setBrightnessSmooth(float brightness, float deltaTime, float lambda = 30.f) {
		if (brightness < value) {
			// Fade out light
			value += (brightness - value) * lambda * deltaTime;
		}
		else {
			// Immediately illuminate light
			value = brightness;
		}
	}
	/** DEPRECATED Alias for setBrightnessSmooth() */
	void setSmoothBrightness(float brightness, float deltaTime) {
		setBrightnessSmooth(brightness, deltaTime);
	}

	/** Use `setBrightnessSmooth(brightness, sampleTime * frames)` instead. */
	DEPRECATED void setBrightnessSmooth(float brightness, int frames = 1) {
		setBrightnessSmooth(brightness, frames / 44100.f);
	}
};


} // namespace engine
} // namespace rack
