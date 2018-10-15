#include "app.hpp"
#include "engine.hpp"


namespace rack {


void ModuleLightWidget::step() {
	assert(module);
	assert(module->lights.size() >= firstLightId + baseColors.size());
	std::vector<float> values(baseColors.size());

	for (size_t i = 0; i < baseColors.size(); i++) {
		float value = module->lights[firstLightId + i].getBrightness();
		value = clamp(value, 0.f, 1.f);
		values[i] = value;
	}
	setValues(values);
}


} // namespace rack
