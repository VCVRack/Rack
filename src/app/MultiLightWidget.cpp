#include "app.hpp"


namespace rack {


void MultiLightWidget::addBaseColor(NVGcolor baseColor) {
	baseColors.push_back(baseColor);
}

void MultiLightWidget::setValues(const std::vector<float> &values) {
	assert(values.size() == baseColors.size());
	color = nvgRGBf(0, 0, 0);
	for (size_t i = 0; i < baseColors.size(); i++) {
		NVGcolor c = baseColors[i];
		float value = values[i];
		color.r += c.r * value;
		color.g += c.g * value;
		color.b += c.b * value;
	}
}


} // namespace rack
