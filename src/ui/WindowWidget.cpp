#include "ui.hpp"


namespace rack {


void WindowWidget::draw(NVGcontext *vg) {
	bndNodeBackground(vg, 0.0, 0.0, box.size.x, box.size.y, BND_DEFAULT, -1, title.c_str(), bndGetTheme()->backgroundColor);
	Widget::draw(vg);
}

void WindowWidget::onDragMove(EventDragMove &e) {
	box.pos = box.pos.plus(e.mouseRel);
}


} // namespace rack
