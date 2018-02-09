#include "ui.hpp"
#include "window.hpp"


namespace rack {


void ScrollBar::draw(NVGcontext *vg) {
	bndScrollBar(vg, 0.0, 0.0, box.size.x, box.size.y, state, offset, size);
}

void ScrollBar::onDragStart(EventDragStart &e) {
	state = BND_ACTIVE;
	windowCursorLock();
}

void ScrollBar::onDragMove(EventDragMove &e) {
	ScrollWidget *scrollWidget = dynamic_cast<ScrollWidget*>(parent);
	assert(scrollWidget);
	if (orientation == HORIZONTAL)
		scrollWidget->offset.x += e.mouseRel.x;
	else
		scrollWidget->offset.y += e.mouseRel.y;
}

void ScrollBar::onDragEnd(EventDragEnd &e) {
	state = BND_DEFAULT;
	windowCursorUnlock();
}


} // namespace rack
