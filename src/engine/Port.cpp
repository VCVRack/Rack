#include "engine/Port.hpp"


namespace rack {
namespace engine {


void Port::step() {
	// Set plug lights
	if (!isConnected() || getChannels() == 0) {
		plugLights[0].setBrightness(0.f);
		plugLights[1].setBrightness(0.f);
		plugLights[2].setBrightness(0.f);
	}
	else if (getChannels() == 1) {
		float v = getVoltage() / 10.f;
		plugLights[0].setBrightnessSmooth(v);
		plugLights[1].setBrightnessSmooth(-v);
		plugLights[2].setBrightness(0.f);
	}
	else {
		float v2 = 0.f;
		for (int c = 0; c < getChannels(); c++) {
			v2 += std::pow(getVoltage(c), 2);
		}
		float v = std::sqrt(v2) / 10.f;
		plugLights[0].setBrightness(0.f);
		plugLights[1].setBrightness(0.f);
		plugLights[2].setBrightnessSmooth(v);
	}
}


} // namespace engine
} // namespace rack
