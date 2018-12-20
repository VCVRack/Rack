#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"


namespace rack {


/** Deletes itself from parent when clicked */
struct MenuOverlay : OpaqueWidget {
	void step() override {
		// Adopt parent's size
		box.size = parent->box.size;

		// Fit all children in the box
		for (Widget *child : children) {
			child->box = child->box.nudge(box.zeroPos());
		}

		Widget::step();
	}

	void onButton(event::Button &e) override {
		OpaqueWidget::onButton(e);

		if (e.target == this && e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
			requestedDelete = true;
		}
	}

	void onHoverKey(event::HoverKey &e) override {
		OpaqueWidget::onHoverKey(e);

		if (e.target == this && e.action == GLFW_PRESS && e.key == GLFW_KEY_ESCAPE) {
			requestedDelete = true;
		}
	}
};


} // namespace rack
