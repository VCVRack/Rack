#pragma once
#include "common.hpp"


namespace rack {


struct Light {
	/** The mean-square of the brightness
	Unstable API. Use set/getBrightness().
	*/
	float value = 0.f;

	void setBrightness(float brightness) {
		value = (brightness > 0.f) ? std::pow(brightness, 2) : 0.f;
	}

	float getBrightness() {
		return std::sqrt(value);
	}

	/** Emulates slow fall (but immediate rise) of LED brightness.
	`frames` rescales the timestep. For example, if your module calls this method every 16 frames, use 16.f.
	*/
	void setBrightnessSmooth(float brightness, float frames = 1.f) {
		float v = (brightness > 0.f) ? std::pow(brightness, 2) : 0.f;
		if (v < value) {
			// Fade out light with lambda = framerate
			// Use 44.1k here to avoid the call to Engine::getSampleRate().
			// This is close enough to look okay up to 96k
			value += (v - value) * frames * 120.f / 44100.f;
		}
		else {
			// Immediately illuminate light
			value = v;
		}
	}
};


} // namespace rack
