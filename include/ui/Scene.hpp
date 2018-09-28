#pragma once

#include "widgets.hpp"
#include "ui/Menu.hpp"


namespace rack {


struct Menu;


struct Scene : OpaqueWidget {
	Widget *overlay = NULL;

	/** Takes ownership of `w` */
	void setOverlay(Widget *w) {
		if (overlay) {
			removeChild(overlay);
			delete overlay;
			overlay = NULL;
		}
		if (w) {
			addChild(w);
			overlay = w;
			overlay->box.pos = math::Vec();
		}
	}

	Menu *createMenu();

	void step() override {
		if (overlay) {
			overlay->box.pos = math::Vec(0, 0);
			overlay->box.size = box.size;
		}

		Widget::step();
	}
};


extern Scene *gScene;


} // namespace rack


#include "ui/MenuOverlay.hpp"


namespace rack {


inline Menu *Scene::createMenu() {
	// Get relative position of the click
	MenuOverlay *overlay = new MenuOverlay();
	Menu *menu = new Menu();
	menu->box.pos = gMousePos;

	overlay->addChild(menu);
	gScene->setOverlay(overlay);

	return menu;
}


} // namespace rack
