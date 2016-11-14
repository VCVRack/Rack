#include "../5V.hpp"


void MenuItem::draw(NVGcontext *vg) {
	bndMenuItem(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, state, -1, text.c_str());
}

void MenuItem::onMouseEnter() {
	state = BND_HOVER;
}

void MenuItem::onMouseLeave() {
	state = BND_DEFAULT;
}

void MenuItem::onMouseUp(int button) {
	onAction();
	// Remove overlay from scene
	// HACK
	Widget *overlay = parent->parent;
	assert(overlay);
	if (overlay->parent) {
		overlay->parent->removeChild(overlay);
	}
	delete overlay;
}
