#include <app/MultiLightWidget.hpp>
#include <color.hpp>


namespace rack {
namespace app {


int MultiLightWidget::getNumColors() {
	return baseColors.size();
}


void MultiLightWidget::addBaseColor(NVGcolor baseColor) {
	baseColors.push_back(baseColor);
}

void MultiLightWidget::setBrightnesses(const std::vector<float>& brightnesses) {
	assert(brightnesses.size() == baseColors.size());
	color = nvgRGBAf(0, 0, 0, 0);
	for (size_t i = 0; i < baseColors.size(); i++) {
		NVGcolor c = baseColors[i];
		c.a *= math::clamp(brightnesses[i], 0.f, 1.f);
		color = color::screen(color, c);
	}
	color = color::clamp(color);
}


} // namespace app
} // namespace rack
