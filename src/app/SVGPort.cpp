#include "app/SVGPort.hpp"


namespace rack {


SVGPort::SVGPort() {
	fb = new FramebufferWidget;
	addChild(fb);

	shadow = new CircularShadow;
	fb->addChild(shadow);
	// Avoid breakage if plugins fail to call setSVG()
	// In that case, just disable the shadow.
	shadow->box.size = math::Vec();

	sw = new SVGWidget;
	fb->addChild(sw);
}

void SVGPort::setSVG(std::shared_ptr<SVG> svg) {
	sw->setSVG(svg);
	fb->box.size = sw->box.size;
	box.size = sw->box.size;
	shadow->box.size = sw->box.size;
	// Move shadow downward by 10%
	shadow->box.pos = math::Vec(0, sw->box.size.y * 0.10);
	// shadow->box = shadow->box.grow(math::Vec(2, 2));
	fb->dirty = true;
}


} // namespace rack
