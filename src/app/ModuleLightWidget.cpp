#include "app/ModuleLightWidget.hpp"


namespace rack {


void ModuleLightWidget::step() {
	if (!module)
		return;

	assert(module->lights.size() >= firstLightId + baseColors.size());
	std::vector<float> brightnesses(baseColors.size());

	for (size_t i = 0; i < baseColors.size(); i++) {
		float brightness = module->lights[firstLightId + i].getBrightness();
		if (!std::isfinite(brightness))
			brightness = 0.f;
		// Because LEDs are nonlinear, this seems to look more natural.
		brightness = std::sqrt(brightness);
		brightness = math::clamp(brightness, 0.f, 1.f);
		brightnesses[i] = brightness;
	}
	setBrightnesses(brightnesses);
}


} // namespace rack
