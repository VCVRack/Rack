#pragma once
#include "ui/common.hpp"
#include "ui/MenuOverlay.hpp"


namespace rack {


#define BND_LABEL_FONT_SIZE 13


struct MenuItem : MenuEntry {
	std::string text;
	std::string rightText;

	void draw(NVGcontext *vg) override {
		// Get state
		BNDwidgetState state = (event::gContext->hoveredWidget == this) ? BND_HOVER : BND_DEFAULT;
		Menu *parentMenu = dynamic_cast<Menu*>(parent);
		if (parentMenu && parentMenu->activeEntry == this) {
			state = BND_ACTIVE;
		}

		bndMenuItem(vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, text.c_str());

		float x = box.size.x - bndLabelWidth(vg, -1, rightText.c_str());
		NVGcolor rightColor = (state == BND_DEFAULT) ? bndGetTheme()->menuTheme.textColor : bndGetTheme()->menuTheme.textSelectedColor;
		bndIconLabelValue(vg, x, 0.0, box.size.x, box.size.y, -1, rightColor, BND_LEFT, BND_LABEL_FONT_SIZE, rightText.c_str(), NULL);
	}

	void step() override {
		// Add 10 more pixels because measurements on high-DPI screens are sometimes too small for some reason
		const float rightPadding = 10.0;
		// HACK use gVg from the window.
		// All this does is inspect the font, so it shouldn't modify gVg and should work when called from a FramebufferWidget for example.
		box.size.x = bndLabelWidth(gVg, -1, text.c_str()) + bndLabelWidth(gVg, -1, rightText.c_str()) + rightPadding;
		Widget::step();
	}

	virtual Menu *createChildMenu() {return NULL;}

	void onEnter(event::Enter &e) override {
		Menu *parentMenu = dynamic_cast<Menu*>(parent);
		if (!parentMenu)
			return;

		parentMenu->activeEntry = NULL;

		// Try to create child menu
		Menu *childMenu = createChildMenu();
		if (childMenu) {
			parentMenu->activeEntry = this;
			childMenu->box.pos = parent->box.pos.plus(box.getTopRight());
		}
		parentMenu->setChildMenu(childMenu);
	}

	void onDragDrop(event::DragDrop &e) override {
		if (e.origin != this)
			return;

		event::Action eAction;
		// Consume event by default, but allow action to un-consume it to prevent the menu from being removed.
		eAction.target = this;
		onAction(eAction);
		if (!eAction.target)
			return;

		Widget *overlay = getAncestorOfType<MenuOverlay>();
		overlay->requestedDelete = true;
	}
};


} // namespace rack
