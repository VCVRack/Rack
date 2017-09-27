#include "widgets.hpp"


namespace rack {

void MenuOverlay::onDragDrop(Widget *origin) {
	if (origin == this) {
		// deletes `this`
		gScene->setOverlay(NULL);
	}
}

Widget *MenuOverlay::onHoverKey(Vec pos, int key) {
	Widget *w = Widget::onHoverKey(pos, key);
	if (w) return w;
	// Steal all keys
	return this;
}


} // namespace rack
