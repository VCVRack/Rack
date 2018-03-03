#include "ui.hpp"


namespace rack {

Menu::~Menu() {
	setChildMenu(NULL);
}

void Menu::setChildMenu(Menu *menu) {
	if (childMenu) {
		if (childMenu->parent)
			childMenu->parent->removeChild(childMenu);
		delete childMenu;
		childMenu = NULL;
	}
	if (menu) {
		childMenu = menu;
		assert(parent);
		parent->addChild(childMenu);
	}
}

void Menu::step() {
	Widget::step();

	// Set positions of children
	box.size = Vec(0, 0);
	for (Widget *child : children) {
		if (!child->visible)
			continue;
		// Increment height, set position of child
		child->box.pos = Vec(0, box.size.y);
		box.size.y += child->box.size.y;
		// Increase width based on maximum width of child
		if (child->box.size.x > box.size.x) {
			box.size.x = child->box.size.x;
		}
	}

	// Resize widths of children
	for (Widget *child : children) {
		child->box.size.x = box.size.x;
	}
}

void Menu::draw(NVGcontext *vg) {
	bndMenuBackground(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE);
	Widget::draw(vg);
}


void Menu::onScroll(EventScroll &e) {
	if (!parent)
		return;
	if (!parent->box.contains(box))
		box.pos.y += e.scrollRel.y;
	e.consumed = true;
}


} // namespace rack
