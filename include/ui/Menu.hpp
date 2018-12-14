#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/common.hpp"
#include "ui/MenuEntry.hpp"


namespace rack {


struct Menu : OpaqueWidget {
	Menu *parentMenu = NULL;
	Menu *childMenu = NULL;
	/** The entry which created the child menu */
	MenuEntry *activeEntry = NULL;

	Menu() {
		box.size = Vec(0, 0);
	}

	~Menu() {
		setChildMenu(NULL);
	}

	/** Deprecated. Just use addChild(child) instead */
	DEPRECATED void pushChild(Widget *child) {
		addChild(child);
	}

	void setChildMenu(Menu *menu) {
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

	void step() override {
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

	void draw(NVGcontext *vg) override {
		bndMenuBackground(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE);
		Widget::draw(vg);
	}

	void onHoverScroll(event::HoverScroll &e) override {
		if (!parent)
			return;
		if (!parent->box.contains(box))
			box.pos.y += e.scrollDelta.y;
		// e.consumed = true;
	}
};


} // namespace rack
