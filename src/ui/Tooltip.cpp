#include "ui/Tooltip.hpp"
#include "app.hpp"
#include "window.hpp"


namespace rack {


void Tooltip::step() {
	// Wrap size to contents
	box.size.x = bndLabelWidth(app()->window->vg, -1, text.c_str()) + 10.0;
	box.size.y = bndLabelHeight(app()->window->vg, -1, text.c_str(), INFINITY);
	Widget::step();
}

void Tooltip::draw(const DrawContext &ctx) {
	bndTooltipBackground(ctx.vg, 0.0, 0.0, box.size.x, box.size.y);
	bndMenuLabel(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
	Widget::draw(ctx);
}


} // namespace rack
