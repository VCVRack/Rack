#include "global_pre.hpp"
#include "ui.hpp"
#include "window.hpp"
#include "global_ui.hpp"


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
	menu->box.pos = global_ui->window.gMousePos;
   printf("xxx Scene::createMenu: box.size=(%f; %f)\n", box.size.x, box.size.y);

	overlay->addChild(menu);
	global_ui->ui.gScene->setOverlay(overlay);

	return menu;
}

void Scene::step() {
	if (overlay) {
		overlay->box.pos = Vec(0, 0);
		overlay->box.size = box.size;
	}

	Widget::step();
}


} // namespace rack
