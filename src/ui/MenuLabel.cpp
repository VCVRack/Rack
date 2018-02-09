#include "ui.hpp"
#include "window.hpp"


namespace rack {


void MenuLabel::draw(NVGcontext *vg) {
	bndMenuLabel(vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
}

void MenuLabel::step() {
	// Add 10 more pixels because Retina measurements are sometimes too small
	const float rightPadding = 10.0;
	// HACK use gVg from the window.
	box.size.x = bndLabelWidth(gVg, -1, text.c_str()) + rightPadding;
	Widget::step();
}


} // namespace rack
