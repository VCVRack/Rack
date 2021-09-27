#include <widget/SvgWidget.hpp>
#include <context.hpp>


namespace rack {
namespace widget {


SvgWidget::SvgWidget() {
	box.size = math::Vec();
}


void SvgWidget::wrap() {
	if (svg) {
		box.size = svg->getSize();
	}
	else {
		box.size = math::Vec();
	}
}


void SvgWidget::setSvg(std::shared_ptr<window::Svg> svg) {
	this->svg = svg;
	wrap();
}


void SvgWidget::draw(const DrawArgs& args) {
	if (!svg)
		return;

	window::svgDraw(args.vg, svg->handle);
}


} // namespace widget
} // namespace rack
