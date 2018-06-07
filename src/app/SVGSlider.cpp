#include "app.hpp"


namespace rack {


SVGSlider::SVGSlider() {
	background = new SVGWidget();
	addChild(background);

	handle = new SVGWidget();
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
	if (dirty) {
		// Interpolate handle position
		handle->box.pos = Vec(rescale(value, minValue, maxValue, minHandlePos.x, maxHandlePos.x), rescale(value, minValue, maxValue, minHandlePos.y, maxHandlePos.y));
	}
	FramebufferWidget::step();
}

void SVGSlider::onChange(EventChange &e) {
	dirty = true;
	Knob::onChange(e);
}


} // namespace rack
