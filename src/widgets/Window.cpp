#include "widgets.hpp"


namespace rack {


void Window::draw(NVGcontext *vg) {
	bndNodeBackground(vg, 0.0, 0.0, box.size.x, box.size.y, BND_DEFAULT, -1, title.c_str(), bndGetTheme()->backgroundColor);
	Widget::draw(vg);
}

void Window::onDragMove(EventDragMove &e) {
	box.pos = box.pos.plus(e.mouseRel);
}


} // namespace rack
