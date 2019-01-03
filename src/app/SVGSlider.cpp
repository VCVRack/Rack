#include "app/SVGSlider.hpp"


namespace rack {


SVGSlider::SVGSlider() {
	background = new SVGWidget;
	addChild(background);

	handle = new SVGWidget;
	addChild(handle);

	speed = 2.0;
}

void SVGSlider::setSVGs(std::shared_ptr<SVG> backgroundSVG, std::shared_ptr<SVG> handleSVG) {
	background->setSVG(backgroundSVG);
	box.size = background->box.size;
	if (handleSVG) {
		handle->setSVG(handleSVG);
	}
}

void SVGSlider::step() {
	Knob::step();
	FramebufferWidget::step();
}

void SVGSlider::onChange(const event::Change &e) {
	if (quantity) {
		// Interpolate handle position
		float v = quantity->getScaledValue();
		handle->box.pos = math::Vec(
			math::rescale(v, 0.f, 1.f, minHandlePos.x, maxHandlePos.x),
			math::rescale(v, 0.f, 1.f, minHandlePos.y, maxHandlePos.y));
		dirty = true;
	}
	ParamWidget::onChange(e);
}


} // namespace rack
