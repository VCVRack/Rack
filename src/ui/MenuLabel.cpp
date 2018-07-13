#include "global_pre.hpp"
#include "ui.hpp"
#include "window.hpp"
#include "global_ui.hpp"


namespace rack {


void MenuLabel::draw(NVGcontext *vg) {
   // printf("xxx drawMenuLabel: text=\"%s\" box.size=(%f; %f)\n", text.c_str(), box.size.x, box.size.y);
   bndMenuLabel(vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
}

void MenuLabel::step() {
	// Add 10 more pixels because Retina measurements are sometimes too small
	const float rightPadding = 10.0;
	// HACK use gVg from the window.
   // printf("xxx MenuLabel::step: x bndLabelWidth text=\"%s\"\n", text.c_str());
	box.size.x = bndLabelWidth(global_ui->window.gVg, -1, text.c_str()) + rightPadding;
   // printf("xxx MenuLabel::step: => bndLabelWidth = %f\n", box.size.x);
	Widget::step();
}


} // namespace rack
