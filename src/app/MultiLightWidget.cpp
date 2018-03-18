#include "app.hpp"
#include "util/color.hpp"


namespace rack {


void MultiLightWidget::addBaseColor(NVGcolor baseColor) {
	baseColors.push_back(baseColor);
}

void MultiLightWidget::setValues(const std::vector<float> &values) {
	assert(values.size() == baseColors.size());
	color = nvgRGBf(0, 0, 0);
	for (size_t i = 0; i < baseColors.size(); i++) {
		NVGcolor c = baseColors[i];
		c = colorMult(c, values[i]);
		color = colorPlus(color, c);
	}
	color = colorClip(color);
}


} // namespace rack
