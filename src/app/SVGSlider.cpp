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
		handle->box.pos = math::Vec(math::rescale(value, minValue, maxValue, minHandlePos.x, maxHandlePos.x), math::rescale(value, minValue, maxValue, minHandlePos.y, maxHandlePos.y));
	}
	FramebufferWidget::step();
}

void SVGSlider::on(event::Change &e) {
	dirty = true;
	ParamWidget::on(e);
}


} // namespace rack
