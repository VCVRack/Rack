#include "app/SVGSlider.hpp"


namespace rack {


SVGSlider::SVGSlider() {
	fb = new FramebufferWidget;
	addChild(fb);

	background = new SVGWidget;
	fb->addChild(background);

	handle = new SVGWidget;
	fb->addChild(handle);

	speed = 2.0;
}

void SVGSlider::setBackgroundSVG(std::shared_ptr<SVG> backgroundSVG) {
	background->setSVG(backgroundSVG);
	fb->box.size = background->box.size;
	box.size = background->box.size;
}

void SVGSlider::setHandleSVG(std::shared_ptr<SVG> handleSVG) {
	handle->setSVG(handleSVG);
	fb->dirty = true;
}

void SVGSlider::onChange(const event::Change &e) {
	if (paramQuantity) {
		// Interpolate handle position
		float v = paramQuantity->getScaledValue();
		handle->box.pos = math::Vec(
			math::rescale(v, 0.f, 1.f, minHandlePos.x, maxHandlePos.x),
			math::rescale(v, 0.f, 1.f, minHandlePos.y, maxHandlePos.y));
		fb->dirty = true;
	}
	ParamWidget::onChange(e);
}


} // namespace rack
