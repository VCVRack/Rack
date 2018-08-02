#include "app.hpp"
#include "color.hpp"


namespace rack {


void MultiLightWidget::addBaseColor(NVGcolor baseColor) {
	baseColors.push_back(baseColor);
}

void MultiLightWidget::setValues(const std::vector<float> &values) {
	assert(values.size() == baseColors.size());
	color = nvgRGBAf(0, 0, 0, 0);
	for (size_t i = 0; i < baseColors.size(); i++) {
		NVGcolor c = baseColors[i];
		c.a *= math::clamp(values[i], 0.f, 1.f);
		color = color::screen(color, c);
	}
	color = color::clip(color);
}


} // namespace rack
