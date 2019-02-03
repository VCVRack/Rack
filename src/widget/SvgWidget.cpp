#include "widget/SvgWidget.hpp"
#include "app.hpp"


namespace rack {
namespace widget {


void SvgWidget::wrap() {
	if (svg && svg->handle) {
		box.size = math::Vec(svg->handle->width, svg->handle->height);
	}
	else {
		box.size = math::Vec();
	}
}

void SvgWidget::setSvg(std::shared_ptr<Svg> svg) {
	this->svg = svg;
	wrap();
}

void SvgWidget::draw(const DrawContext &ctx) {
	if (svg && svg->handle) {
		svgDraw(ctx.vg, svg->handle);
	}
}


} // namespace widget
} // namespace rack
