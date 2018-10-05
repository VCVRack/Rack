#pragma once

#include "widgets.hpp"


namespace rack {


/** Deletes itself from parent when clicked */
struct MenuOverlay : OpaqueWidget {
	void step() override {
		Widget::step();

		// Adopt parent's size
		box.size = parent->box.size;

		// Fit all children in the box
		for (Widget *child : children) {
			child->box = child->box.nudge(box.zeroPos());
		}
	}

	void on(event::Button &e) override {
		EventWidget::on(e);

		if (!e.target) {
			e.target = this;
			requestedDelete = true;
		}
	}

	void on(event::HoverKey &e) override {
		switch (e.key) {
			case GLFW_KEY_ESCAPE: {
				e.target = this;
				requestedDelete = true;
				return;
			} break;
		}

		EventWidget::on(e);
	}
};


} // namespace rack
