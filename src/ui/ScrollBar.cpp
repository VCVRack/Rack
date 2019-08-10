#include <ui/ScrollBar.hpp>
#include <ui/ScrollWidget.hpp>
#include <app.hpp>
#include <window.hpp>


namespace rack {
namespace ui {


ScrollBar::ScrollBar() {
	box.size = math::Vec(BND_SCROLLBAR_WIDTH, BND_SCROLLBAR_HEIGHT);
}

void ScrollBar::draw(const DrawArgs& args) {
	BNDwidgetState state = BND_DEFAULT;
	if (APP->event->hoveredWidget == this)
		state = BND_HOVER;
	if (APP->event->draggedWidget == this)
		state = BND_ACTIVE;

	bndScrollBar(args.vg, 0.0, 0.0, box.size.x, box.size.y, state, offset, size);
}

void ScrollBar::onDragStart(const event::DragStart& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	APP->window->cursorLock();
}

void ScrollBar::onDragMove(const event::DragMove& e) {
	const float sensitivity = 1.f;

	ScrollWidget* scrollWidget = dynamic_cast<ScrollWidget*>(parent);
	assert(scrollWidget);
	if (orientation == HORIZONTAL)
		scrollWidget->offset.x += sensitivity * e.mouseDelta.x;
	else
		scrollWidget->offset.y += sensitivity * e.mouseDelta.y;
}

void ScrollBar::onDragEnd(const event::DragEnd& e) {
	APP->window->cursorUnlock();
}


} // namespace ui
} // namespace rack
