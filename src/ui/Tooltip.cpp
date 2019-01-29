#include "ui/Tooltip.hpp"
#include "app.hpp"
#include "window.hpp"


namespace rack {
namespace ui {


void Tooltip::step() {
	// Wrap size to contents
	box.size.x = bndLabelWidth(APP->window->vg, -1, text.c_str()) + 10.0;
	box.size.y = bndLabelHeight(APP->window->vg, -1, text.c_str(), INFINITY);
	widget::Widget::step();
}

void Tooltip::draw(const widget::DrawContext &ctx) {
	bndTooltipBackground(ctx.vg, 0.0, 0.0, box.size.x, box.size.y);
	bndMenuLabel(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
	widget::Widget::draw(ctx);
}


} // namespace ui
} // namespace rack
