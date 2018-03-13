#include "app.hpp"


namespace rack {


SVGPort::SVGPort() {
	shadow = new CircularShadow();
	addChild(shadow);
	// Avoid breakage if plugins fail to call setSVG()
	// In that case, just disable the shadow.
	shadow->box.size = Vec();

	background = new SVGWidget();
	addChild(background);
}

void SVGPort::setSVG(std::shared_ptr<SVG> svg) {
	background->setSVG(svg);
	box.size = background->box.size;
	shadow->box.size = background->box.size;
	shadow->box.pos = Vec(0, background->box.size.y * 0.1);
	// shadow->box = shadow->box.grow(Vec(2, 2));
}

void SVGPort::draw(NVGcontext *vg) {
	Port::draw(vg);
	FramebufferWidget::draw(vg);
}


} // namespace rack
