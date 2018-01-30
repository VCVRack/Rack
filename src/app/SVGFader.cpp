#include "app.hpp"


namespace rack {


SVGFader::SVGFader() {
	background = new SVGWidget();
	addChild(background);

	handle = new SVGWidget();
	addChild(handle);
}

void SVGFader::step() {
	if (dirty) {
		// Update handle position
		Vec handlePos = Vec(rescalef(value, minValue, maxValue, minHandlePos.x, maxHandlePos.x), rescalef(value, minValue, maxValue, minHandlePos.y, maxHandlePos.y));
		handle->box.pos = handlePos;
	}
	FramebufferWidget::step();
}

void SVGFader::onChange(EventChange &e) {
	dirty = true;
	Knob::onChange(e);
}


} // namespace rack
