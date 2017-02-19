#include "app.hpp"


namespace rack {


SVGPort::SVGPort() {
	sw = new SVGWidget();
	setScene(sw);
}

void SVGPort::setSVG(std::shared_ptr<SVG> svg) {
	sw->svg = svg;
	sw->wrap();
}

void SVGPort::draw(NVGcontext *vg) {
	Port::draw(vg);
	FramebufferWidget::draw(vg);
}


} // namespace rack
