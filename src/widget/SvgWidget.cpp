#include <widget/SvgWidget.hpp>
#include <context.hpp>


namespace rack {
namespace widget {


void SvgWidget::wrap() {
	if (svg) {
		box.size = svg->getSize();
	}
	else {
		box.size = math::Vec();
	}
}

void SvgWidget::setSvg(std::shared_ptr<Svg> svg) {
	this->svg = svg;
	wrap();
}

void SvgWidget::draw(const DrawArgs& args) {
	if (!svg)
		return;

	svgDraw(args.vg, svg->handle);
}


} // namespace widget
} // namespace rack
