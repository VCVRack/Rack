#include "ui/ScrollBar.hpp"
#include "ui/ScrollWidget.hpp"
#include "context.hpp"
#include "window.hpp"


namespace rack {


static const float SCROLLBAR_SENSITIVITY = 2.f;


ScrollBar::ScrollBar() {
	box.size = math::Vec(BND_SCROLLBAR_WIDTH, BND_SCROLLBAR_HEIGHT);
}

void ScrollBar::draw(NVGcontext *vg) {
	bndScrollBar(vg, 0.0, 0.0, box.size.x, box.size.y, state, offset, size);
}

void ScrollBar::onDragStart(const event::DragStart &e) {
	state = BND_ACTIVE;
	context()->window->cursorLock();
}

void ScrollBar::onDragMove(const event::DragMove &e) {
	ScrollWidget *scrollWidget = dynamic_cast<ScrollWidget*>(parent);
	assert(scrollWidget);
	if (orientation == HORIZONTAL)
		scrollWidget->offset.x += SCROLLBAR_SENSITIVITY * e.mouseDelta.x;
	else
		scrollWidget->offset.y += SCROLLBAR_SENSITIVITY * e.mouseDelta.y;
}

void ScrollBar::onDragEnd(const event::DragEnd &e) {
	state = BND_DEFAULT;
	context()->window->cursorUnlock();
}


} // namespace rack
