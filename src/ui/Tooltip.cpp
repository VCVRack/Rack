#include "ui/Tooltip.hpp"
#include "app.hpp"
#include "window.hpp"


namespace rack {
namespace ui {


void Tooltip::step() {
	// Wrap size to contents
	box.size.x = bndLabelWidth(APP->window->vg, -1, text.c_str()) + 10.0;
	box.size.y = bndLabelHeight(APP->window->vg, -1, text.c_str(), INFINITY);
	Widget::step();
}

void Tooltip::draw(const DrawArgs &args) {
	bndTooltipBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y);
	bndMenuLabel(args.vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
	Widget::draw(args);
}


} // namespace ui
} // namespace rack
