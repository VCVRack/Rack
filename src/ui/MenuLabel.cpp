#include "ui/MenuLabel.hpp"
#include "app.hpp"


namespace rack {


void MenuLabel::draw(const DrawContext &ctx) {
	bndMenuLabel(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
}

void MenuLabel::step() {
	// Add 10 more pixels because Retina measurements are sometimes too small
	const float rightPadding = 10.0;
	// HACK use app()->window->vg from the window.
	box.size.x = bndLabelWidth(app()->window->vg, -1, text.c_str()) + rightPadding;
	Widget::step();
}


} // namespace rack
