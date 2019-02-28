#include "app/ModuleLightWidget.hpp"


namespace rack {
namespace app {


void ModuleLightWidget::step() {
	std::vector<float> brightnesses(baseColors.size());

	if (module) {
		assert(module->lights.size() >= firstLightId + baseColors.size());

		for (size_t i = 0; i < baseColors.size(); i++) {
			float brightness = module->lights[firstLightId + i].getBrightness();
			if (!std::isfinite(brightness))
				brightness = 0.f;
			// Because LEDs are nonlinear, this seems to look more natural.
			brightness = std::sqrt(brightness);
			brightness = math::clamp(brightness, 0.f, 1.f);
			brightnesses[i] = brightness;
		}
	}
	else {
		// Turn all lights on
		for (size_t i = 0; i < baseColors.size(); i++) {
			brightnesses[i] = 1.f;
		}
	}

	setBrightnesses(brightnesses);
}


} // namespace app
} // namespace rack
