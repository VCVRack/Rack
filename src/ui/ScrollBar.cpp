#include <ui/ScrollBar.hpp>
#include <ui/ScrollWidget.hpp>
#include <context.hpp>
#include <window.hpp>


namespace rack {
namespace ui {


// Internal not currently used


ScrollBar::ScrollBar() {
	box.size = math::Vec(BND_SCROLLBAR_WIDTH, BND_SCROLLBAR_HEIGHT);
}


ScrollBar::~ScrollBar() {
}


void ScrollBar::draw(const DrawArgs& args) {
	ScrollWidget* sw = dynamic_cast<ScrollWidget*>(parent);
	assert(sw);

	BNDwidgetState state = BND_DEFAULT;
	if (APP->event->getHoveredWidget() == this)
		state = BND_HOVER;
	if (APP->event->getDraggedWidget() == this)
		state = BND_ACTIVE;

	float offsetBound = sw->containerBox.size.get(vertical) - sw->box.size.get(vertical);
	// The handle position relative to the scrollbar. [0, 1]
	float scrollBarOffset = (sw->offset.get(vertical) - sw->containerBox.pos.get(vertical)) / offsetBound;
	// The handle size relative to the scrollbar. [0, 1]
	float scrollBarSize = sw->box.size.get(vertical) / sw->containerBox.size.get(vertical);
	bndScrollBar(args.vg, 0.0, 0.0, box.size.x, box.size.y, state, scrollBarOffset, scrollBarSize);
}


void ScrollBar::onDragStart(const event::DragStart& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		APP->window->cursorLock();
	}
}


void ScrollBar::onDragEnd(const event::DragEnd& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
		APP->window->cursorUnlock();
	}
}


void ScrollBar::onDragMove(const event::DragMove& e) {
	ScrollWidget* sw = dynamic_cast<ScrollWidget*>(parent);
	assert(sw);

	// TODO
	// float offsetBound = sw->containerBox.size.get(vertical) - sw->box.size.get(vertical);
	// float scrollBarSize = sw->box.size.get(vertical) / sw->containerBox.size.get(vertical);

	const float sensitivity = 1.f;
	float offsetDelta = e.mouseDelta.get(vertical);
	offsetDelta *= sensitivity;
	sw->offset.get(vertical) += offsetDelta;
}


} // namespace ui
} // namespace rack
