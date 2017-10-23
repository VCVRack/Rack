#include "widgets.hpp"


namespace rack {

Menu::~Menu() {
	setChildMenu(NULL);
}

void Menu::pushChild(Widget *child) {
	child->box.pos = Vec(0, box.size.y);
	addChild(child);
	box.size.y += child->box.size.y;
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
	// Try to fit into the parent's box
	if (parent)
		box = box.clamp(Rect(Vec(0, 0), parent->box.size));

	Widget::step();

	// Resize the width to the widest child
	for (Widget *child : children) {
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


bool Menu::onScrollOpaque(Vec scrollRel) {
	if (!parent)
		return true;
	if (!parent->box.contains(box))
		box.pos = box.pos.plus(scrollRel);
	return true;
}


} // namespace rack
