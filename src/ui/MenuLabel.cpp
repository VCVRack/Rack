#include <ui/MenuLabel.hpp>
#include <context.hpp>


namespace rack {
namespace ui {


void MenuLabel::draw(const DrawArgs& args) {
	bndMenuLabel(args.vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
}

void MenuLabel::step() {
	// Add 10 more pixels because Retina measurements are sometimes too small
	const float rightPadding = 10.0;
	// HACK use APP->window->vg from the window.
	box.size.x = bndLabelWidth(APP->window->vg, -1, text.c_str()) + rightPadding;
	Widget::step();
}


} // namespace ui
} // namespace rack
