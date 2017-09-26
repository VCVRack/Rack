#include "widgets.hpp"


namespace rack {

void MenuOverlay::onDragDrop(Widget *origin) {
	if (origin == this) {
		// deletes `this`
		gScene->setOverlay(NULL);
	}
}


} // namespace rack
