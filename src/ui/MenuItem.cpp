#include "ui.hpp"
#include "window.hpp"


namespace rack {


#define BND_LABEL_FONT_SIZE 13


void MenuItem::draw(NVGcontext *vg) {
	// Get state
	BNDwidgetState state = (gHoveredWidget == this) ? BND_HOVER : BND_DEFAULT;
	Menu *parentMenu = dynamic_cast<Menu*>(parent);
	if (parentMenu && parentMenu->activeEntry == this) {
		state = BND_ACTIVE;
	}

	bndMenuItem(vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, text.c_str());

	float x = box.size.x - bndLabelWidth(vg, -1, rightText.c_str());
	NVGcolor rightColor = (state == BND_DEFAULT) ? bndGetTheme()->menuTheme.textColor : bndGetTheme()->menuTheme.textSelectedColor;
	bndIconLabelValue(vg, x, 0.0, box.size.x, box.size.y, -1, rightColor, BND_LEFT, BND_LABEL_FONT_SIZE, rightText.c_str(), NULL);
}

void MenuItem::step() {
	// Add 10 more pixels because measurements on high-DPI screens are sometimes too small for some reason
	const float rightPadding = 10.0;
	// HACK use gVg from the window.
	// All this does is inspect the font, so it shouldn't modify gVg and should work when called from a FramebufferWidget for example.
	box.size.x = bndLabelWidth(gVg, -1, text.c_str()) + bndLabelWidth(gVg, -1, rightText.c_str()) + rightPadding;
	Widget::step();
}

void MenuItem::onMouseEnter(EventMouseEnter &e) {
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

void MenuItem::onDragDrop(EventDragDrop &e) {
	if (e.origin != this)
		return;

	EventAction eAction;
	// Consume event by default, but allow action to un-consume it to prevent the menu from being removed.
	eAction.consumed = true;
	onAction(eAction);
	if (eAction.consumed) {
		// deletes `this`
		gScene->setOverlay(NULL);
	}
}


} // namespace rack
