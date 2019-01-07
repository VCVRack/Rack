#include "ui/MenuOverlay.hpp"


namespace rack {


void MenuOverlay::step() {
	// Adopt parent's size
	box.size = parent->box.size;

	// Fit all children in the box
	for (Widget *child : children) {
		child->box = child->box.nudge(box.zeroPos());
	}

	Widget::step();
}

void MenuOverlay::onButton(const event::Button &e) {
	OpaqueWidget::onButton(e);

	if (e.getConsumed() == this && e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		requestedDelete = true;
	}
}

void MenuOverlay::onHoverKey(const event::HoverKey &e) {
	OpaqueWidget::onHoverKey(e);

	if (e.getConsumed() == this && e.action == GLFW_PRESS && e.key == GLFW_KEY_ESCAPE) {
		requestedDelete = true;
	}
}


} // namespace rack
