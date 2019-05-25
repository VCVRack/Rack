#include <ui/MenuOverlay.hpp>


namespace rack {
namespace ui {


void MenuOverlay::step() {
	// Adopt parent's size
	box.size = parent->box.size;

	// Fit all children in the box
	for (widget::Widget *child : children) {
		child->box = child->box.nudge(box.zeroPos());
	}

	Widget::step();
}

void MenuOverlay::onButton(const event::Button &e) {
	OpaqueWidget::onButton(e);
	if (e.getTarget() != this)
		return;

	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		requestDelete();
		e.consume(this);
	}
}

void MenuOverlay::onHoverKey(const event::HoverKey &e) {
	OpaqueWidget::onHoverKey(e);
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ESCAPE) {
		requestDelete();
		e.consume(this);
	}
}


} // namespace ui
} // namespace rack
