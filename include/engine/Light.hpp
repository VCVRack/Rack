#pragma once
#include "common.hpp"


namespace rack {


struct Light {
	float value = 0.f;
	float getBrightness();
	void setBrightness(float brightness) {
		value = (brightness > 0.f) ? std::pow(brightness, 2) : 0.f;
	}
	/** Emulates slow fall (but immediate rise) of LED brightness.
	`frames` rescales the timestep. For example, if your module calls this method every 16 frames, use 16.f.
	*/
	void setBrightnessSmooth(float brightness, float frames = 1.f);
};


} // namespace rack
