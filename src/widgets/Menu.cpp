#include "widgets.hpp"


namespace rack {

void Menu::pushChild(Widget *child) {
	child->box.pos = Vec(0, box.size.y);
	addChild(child);
	box.size.y += child->box.size.y;
}

void Menu::draw(NVGcontext *vg) {
	// Resize the width to the widest child
	for (Widget *child : children) {
		MenuEntry *menuEntry = dynamic_cast<MenuEntry*>(child);
		float width = menuEntry->computeMinWidth(vg);
		if (width > box.size.x) {
			box.size.x = width;
		}
	}
	// Resize widths of children
	for (Widget *child : children) {
		child->box.size.x = box.size.x;
	}

	bndMenuBackground(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE);

	Widget::draw(vg);
}


bool Menu::onScrollOpaque(Vec scrollRel) {
	if (!parent)
		return true;
	if (!parent->box.contains(box))
		box.pos = box.pos.plus(scrollRel.neg());
	return true;
}


} // namespace rack
