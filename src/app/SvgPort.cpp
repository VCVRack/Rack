#include <app/SvgPort.hpp>


namespace rack {
namespace app {


SvgPort::SvgPort() {
	fb = new widget::FramebufferWidget;
	addChild(fb);

	shadow = new CircularShadow;
	fb->addChild(shadow);
	// Avoid breakage if plugins fail to call setSvg()
	// In that case, just disable the shadow.
	shadow->box.size = math::Vec();

	sw = new widget::SvgWidget;
	fb->addChild(sw);
}

void SvgPort::setSvg(std::shared_ptr<window::Svg> svg) {
	sw->setSvg(svg);
	fb->box.size = sw->box.size;
	box.size = sw->box.size;
	// Move shadow downward by 10%
	shadow->box.size = sw->box.size;
	shadow->box.pos = math::Vec(0, sw->box.size.y * 0.10);

	fb->dirty = true;
}


} // namespace app
} // namespace rack
