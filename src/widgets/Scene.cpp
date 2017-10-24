#include "widgets.hpp"
#include "gui.hpp"


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

Menu *Scene::createMenu() {
	// Get relative position of the click
	MenuOverlay *overlay = new MenuOverlay();
	Menu *menu = new Menu();
	menu->box.pos = gMousePos;

	overlay->addChild(menu);
	gScene->setOverlay(overlay);

	return menu;
}

void Scene::step() {
	if (overlay) {
		overlay->box.size = box.size;
	}

	Widget::step();
}


} // namespace rack
