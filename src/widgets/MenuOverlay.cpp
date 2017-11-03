#include "widgets.hpp"


namespace rack {

void MenuOverlay::onDragDrop(EventDragDrop &e) {
	if (e.origin == this) {
		// deletes `this`
		gScene->setOverlay(NULL);
	}
}

void MenuOverlay::onScroll(EventScroll &e) {
	// Don't recurse children, consume the event
	e.consumed = true;
}

void MenuOverlay::onHoverKey(EventHoverKey &e) {
	// Recurse children but consume the event
	Widget::onHoverKey(e);
	e.consumed = true;
}


} // namespace rack
