#include "rack.hpp"


namespace rack {

void MenuItem::draw(NVGcontext *vg) {
	bndMenuItem(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, state, -1, text.c_str());
}

void MenuItem::onMouseEnter() {
	state = BND_HOVER;
}

void MenuItem::onMouseLeave() {
	state = BND_DEFAULT;
}

void MenuItem::onDragDrop(Widget *origin) {
	if (origin != this)
		return;

	onAction();
	// deletes `this`
	gScene->setOverlay(NULL);
}


} // namespace rack
