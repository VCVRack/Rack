#include "widgets.hpp"


namespace rack {


void MenuOverlay::step() {
	Widget::step();

	// Fit all children in the box
	for (Widget *child : children) {
		child->box = child->box.nudge(Rect(Vec(0, 0), parent->box.size));
	}
}

void MenuOverlay::onDragDrop(EventDragDrop &e) {
	if (e.origin == this) {
		// deletes `this`
		gScene->setOverlay(NULL);
	}
}

void MenuOverlay::onHoverKey(EventHoverKey &e) {
	// Recurse children but consume the event
	Widget::onHoverKey(e);
	e.consumed = true;
}


} // namespace rack
