#include <app/SvgScrew.hpp>


namespace rack {
namespace app {


SvgScrew::SvgScrew() {
	fb = new widget::FramebufferWidget;
	addChild(fb);

	sw = new widget::SvgWidget;
	fb->addChild(sw);
}


void SvgScrew::setSvg(std::shared_ptr<window::Svg> svg) {
	sw->setSvg(svg);
	fb->box.size = sw->box.size;
	box.size = sw->box.size;
}


} // namespace app
} // namespace rack
