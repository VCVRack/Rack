#include "app.hpp"


namespace rack {


SVGPort::SVGPort() {
	background = new SVGWidget();
	addChild(background);
}

void SVGPort::draw(NVGcontext *vg) {
	Port::draw(vg);
	FramebufferWidget::draw(vg);
}


} // namespace rack
