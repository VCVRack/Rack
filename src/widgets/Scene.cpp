#include "widgets.hpp"


namespace rack {

void Scene::setOverlay(Widget *w) {
	if (overlay) {
		removeChild(overlay);
		delete overlay;
		overlay = NULL;
	}
	if (w) {
		addChild(w);
		overlay = w;
		overlay->box.pos = Vec();
	}
}

void Scene::step() {
	if (overlay) {
		overlay->box.size = box.size;
	}

	Widget::step();
}


} // namespace rack
