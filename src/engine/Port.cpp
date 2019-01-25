#include "engine/Port.hpp"


namespace rack {


void Port::step() {
	// Set plug lights
	if (channels == 0) {
		plugLights[0].setBrightness(0.f);
		plugLights[1].setBrightness(0.f);
		plugLights[2].setBrightness(0.f);
	}
	else if (channels == 1) {
		float v = getVoltage() / 10.f;
		plugLights[0].setBrightnessSmooth(v);
		plugLights[1].setBrightnessSmooth(-v);
		plugLights[2].setBrightness(0.f);
	}
	else {
		float v2 = 0.f;
		for (int c = 0; c < channels; c++) {
			v2 += std::pow(getVoltage(c), 2);
		}
		float v = std::sqrt(v2) / 10.f;
		plugLights[0].setBrightness(0.f);
		plugLights[1].setBrightness(0.f);
		plugLights[2].setBrightnessSmooth(v);
	}
}


} // namespace rack
