#pragma once

#include "widgets.hpp"


namespace rack {


/** Deletes itself from parent when clicked */
struct MenuOverlay : OpaqueWidget {
	void step() override {
		Widget::step();

		// Fit all children in the box
		for (Widget *child : children) {
			child->box = child->box.nudge(box.zeroPos());
		}
	}

	void on(event::Button &e) override;
	void on(event::HoverKey &e) override;
};


} // namespace rack


#include "ui/Scene.hpp"


namespace rack {


inline void MenuOverlay::on(event::Button &e) {
	EventWidget::on(e);
	if (!e.target) {
		// deletes `this`
		gScene->setOverlay(NULL);
		e.target = this;
	}
}

inline void MenuOverlay::on(event::HoverKey &e) {
	switch (e.key) {
		case GLFW_KEY_ESCAPE: {
			gScene->setOverlay(NULL);
			// e.consumed = true;
			return;
		} break;
	}

	// if (!e.consumed) {
	// 	// Recurse children but consume the event
	// 	Widget::onHoverKey(e);
	// 	e.consumed = true;
	// }
}


} // namespace rack
