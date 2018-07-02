#include "global_pre.hpp"
#include "ui.hpp"
#include "window.hpp"
#include "global_ui.hpp"


namespace rack {


void MenuOverlay::step() {
	Widget::step();

	// Fit all children in the box
	for (Widget *child : children) {
		child->box = child->box.nudge(box.zeroPos());
	}
}

void MenuOverlay::onMouseDown(EventMouseDown &e) {
	Widget::onMouseDown(e);
	if (!e.consumed) {
		// deletes `this`
		global_ui->ui.gScene->setOverlay(NULL);
		e.consumed = true;
	}
}

void MenuOverlay::onHoverKey(EventHoverKey &e) {
	switch (e.key) {
		case GLFW_KEY_ESCAPE: {
			global_ui->ui.gScene->setOverlay(NULL);
			e.consumed = true;
			return;
		} break;
	}

	if (!e.consumed) {
		// Recurse children but consume the event
		Widget::onHoverKey(e);
		e.consumed = true;
	}
}


} // namespace rack
