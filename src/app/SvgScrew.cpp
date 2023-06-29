#include <app/SvgScrew.hpp>
#include <settings.hpp>


namespace rack {
namespace app {


SvgScrew::SvgScrew() {
	fb = new widget::FramebufferWidget;
	addChild(fb);

	sw = new widget::SvgWidget;
	fb->addChild(sw);
}


void SvgScrew::setSvg(std::shared_ptr<window::Svg> svg) {
	if (sw->svg == svg)
		return;

	sw->setSvg(svg);
	fb->setDirty();

	fb->box.size = sw->box.size;
	box.size = sw->box.size;
}


void ThemedSvgScrew::step() {
	SvgScrew::setSvg(settings::preferDarkPanels ? darkSvg : lightSvg);
	SvgScrew::step();
}


void ThemedSvgScrew::setSvg(std::shared_ptr<window::Svg> lightSvg, std::shared_ptr<window::Svg> darkSvg) {
	this->lightSvg = lightSvg;
	this->darkSvg = darkSvg;
	SvgScrew::setSvg(settings::preferDarkPanels ? darkSvg : lightSvg);
}


} // namespace app
} // namespace rack
