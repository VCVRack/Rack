#include "app.hpp"
#include "engine.hpp"


namespace rack {


void ColorLightWidget::addColor(NVGcolor c) {
	colors.push_back(c);
}

void ColorLightWidget::step() {
	assert(module);
	assert(module->lights.size() >= lightId + colors.size());
	color = nvgRGBf(0, 0, 0);
	for (int i = 0; i < (int)colors.size(); i++) {
		NVGcolor c = colors[i];
		float brightness = module->lights[lightId + i].getBrightness();
		brightness = clampf(brightness, 0.0, 1.0);
		color.r += c.r * brightness;
		color.g += c.g * brightness;
		color.b += c.b * brightness;
	}
}


} // namespace rack
