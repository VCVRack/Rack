#include "widgets.hpp"


namespace rack {

void MenuOverlay::step() {
	// Try to fit all children into the overlay's box
	for (Widget *child : children) {
		child->box = child->box.clamp(Rect(Vec(0, 0), box.size));
	}

	Widget::step();
}

void MenuOverlay::onDragDrop(Widget *origin) {
	if (origin == this) {
		// deletes `this`
		gScene->setOverlay(NULL);
	}
}


} // namespace rack
