#include "widgets.hpp"


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
	onAction(eAction);
	// deletes `this`
	gScene->setOverlay(NULL);
}


} // namespace rack
