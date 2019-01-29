#include "ui/ScrollBar.hpp"
#include "ui/ScrollWidget.hpp"
#include "app.hpp"
#include "window.hpp"


namespace rack {
namespace ui {


static const float SCROLLBAR_SENSITIVITY = 2.f;


ScrollBar::ScrollBar() {
	box.size = math::Vec(BND_SCROLLBAR_WIDTH, BND_SCROLLBAR_HEIGHT);
}

void ScrollBar::draw(const widget::DrawContext &ctx) {
	bndScrollBar(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, state, offset, size);
}

void ScrollBar::onDragStart(const event::DragStart &e) {
	state = BND_ACTIVE;
	APP->window->cursorLock();
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
	APP->window->cursorUnlock();
}


} // namespace ui
} // namespace rack
