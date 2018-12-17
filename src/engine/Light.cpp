#include "engine/Light.hpp"
#include "engine/Engine.hpp"


namespace rack {


float Light::getBrightness() {
	// LEDs are diodes, so don't allow reverse current.
	// For some reason, instead of the RMS, the sqrt of RMS looks better
	return std::pow(std::fmax(0.f, value), 0.25f);
}

void Light::setBrightnessSmooth(float brightness, float frames) {
	float v = (brightness > 0.f) ? std::pow(brightness, 2) : 0.f;
	if (v < value) {
		// Fade out light with lambda = framerate
		value += (v - value) * gEngine->getSampleTime() * frames * 60.f;
	}
	else {
		// Immediately illuminate light
		value = v;
	}
}


} // namespace rack
