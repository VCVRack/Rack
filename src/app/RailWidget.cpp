#include <app/RailWidget.hpp>
#include <context.hpp>
#include <asset.hpp>
#include <widget/SvgWidget.hpp>
#include <widget/FramebufferWidget.hpp>


namespace rack {
namespace app {


struct RailWidget::Internal {
	widget::FramebufferWidget* railFb;
	widget::SvgWidget* railSw;
};


RailWidget::RailWidget() {
	internal = new Internal;

	internal->railFb = new widget::FramebufferWidget;
	// The rail renders fine without oversampling, and it would be too expensive anyway.
	internal->railFb->oversample = 1.0;
	// Don't redraw when the world offset of the rail FramebufferWidget changes its fractional value.
	internal->railFb->dirtyOnSubpixelChange = false;
	addChild(internal->railFb);

	internal->railSw = new widget::SvgWidget;
	internal->railSw->setSvg(window::Svg::load(asset::system("res/ComponentLibrary/Rail.svg")));
	internal->railFb->addChild(internal->railSw);
}


RailWidget::~RailWidget() {
	delete internal;
}


void RailWidget::draw(const DrawArgs& args) {
	if (!internal->railSw->svg)
		return;

	math::Vec tileSize = internal->railSw->svg->getSize().div(RACK_GRID_SIZE).round().mult(RACK_GRID_SIZE);
	if (tileSize.area() == 0.f)
		return;

	math::Vec min = args.clipBox.getTopLeft().div(tileSize).floor().mult(tileSize);
	math::Vec max = args.clipBox.getBottomRight().div(tileSize).ceil().mult(tileSize);

	// Draw the same FramebufferWidget repeatedly as a tile
	math::Vec p;
	for (p.y = min.y; p.y < max.y; p.y += tileSize.y) {
		for (p.x = min.x; p.x < max.x; p.x += tileSize.x) {
			internal->railFb->box.pos = p;
			Widget::drawChild(internal->railFb, args);
		}
	}
}


} // namespace app
} // namespace rack
