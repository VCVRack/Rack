#include "app/ModuleLightWidget.hpp"


namespace rack {
namespace app {


void ModuleLightWidget::step() {
	std::vector<float> brightnesses(baseColors.size());

	if (module) {
		assert(module->lights.size() >= firstLightId + baseColors.size());

		for (size_t i = 0; i < baseColors.size(); i++) {
			float b = module->lights[firstLightId + i].getBrightness();
			if (!std::isfinite(b))
				b = 0.f;
			b = math::clamp(b, 0.f, 1.f);
			// Because LEDs are nonlinear, this seems to look more natural.
			b = std::sqrt(b);
			brightnesses[i] = b;
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
