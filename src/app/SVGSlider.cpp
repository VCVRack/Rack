#include "app.hpp"


namespace rack {


SVGSlider::SVGSlider() {
	background = new SVGWidget();
	addChild(background);

	handle = new SVGWidget();
	addChild(handle);
}

void SVGSlider::step() {
	if (dirty) {
		// Update handle position
		Vec handlePos = Vec(mapf(value, minValue, maxValue, minHandlePos.x, maxHandlePos.x), mapf(value, minValue, maxValue, minHandlePos.y, maxHandlePos.y));
		handle->box.pos = handlePos;
	}
	FramebufferWidget::step();
}

void SVGSlider::onChange() {
	dirty = true;
	ParamWidget::onChange();
}


} // namespace rack
