#include <ui/MenuItem.hpp>
#include <ui/MenuOverlay.hpp>


namespace rack {
namespace ui {


#define BND_LABEL_FONT_SIZE 13


void MenuItem::draw(const DrawArgs& args) {
	BNDwidgetState state = BND_DEFAULT;

	if (APP->event->hoveredWidget == this)
		state = BND_HOVER;

	// Set active state if this MenuItem
	Menu* parentMenu = dynamic_cast<Menu*>(parent);
	if (parentMenu && parentMenu->activeEntry == this)
		state = BND_ACTIVE;

	if (active)
		state = BND_ACTIVE;

	// Main text and background
	if (!disabled)
		bndMenuItem(args.vg, 0.0, 0.0, box.size.x, box.size.y, state, -1, text.c_str());
	else
		bndMenuLabel(args.vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());

	// Right text
	float x = box.size.x - bndLabelWidth(args.vg, -1, rightText.c_str());
	NVGcolor rightColor = (state == BND_DEFAULT && !disabled) ? bndGetTheme()->menuTheme.textColor : bndGetTheme()->menuTheme.textSelectedColor;
	bndIconLabelValue(args.vg, x, 0.0, box.size.x, box.size.y, -1, rightColor, BND_LEFT, BND_LABEL_FONT_SIZE, rightText.c_str(), NULL);
}

void MenuItem::step() {
	// Add 10 more pixels because measurements on high-DPI screens are sometimes too small for some reason
	const float rightPadding = 10.0;
	// HACK use APP->window->vg from the window.
	// All this does is inspect the font, so it shouldn't modify APP->window->vg and should work when called from a widget::FramebufferWidget for example.
	box.size.x = bndLabelWidth(APP->window->vg, -1, text.c_str()) + bndLabelWidth(APP->window->vg, -1, rightText.c_str()) + rightPadding;
	Widget::step();
}

void MenuItem::onEnter(const event::Enter& e) {
	Menu* parentMenu = dynamic_cast<Menu*>(parent);
	if (!parentMenu)
		return;

	parentMenu->activeEntry = NULL;

	// Try to create child menu
	Menu* childMenu = createChildMenu();
	if (childMenu) {
		parentMenu->activeEntry = this;
		childMenu->box.pos = parent->box.pos.plus(box.getTopRight());
	}
	parentMenu->setChildMenu(childMenu);
}

void MenuItem::onDragDrop(const event::DragDrop& e) {
	if (e.origin != this)
		return;
	doAction();
}

void MenuItem::doAction() {
	if (disabled)
		return;

	event::Context cAction;
	event::Action eAction;
	eAction.context = &cAction;
	// Consume event by default, but allow action to un-consume it to prevent the menu from being removed.
	eAction.consume(this);
	onAction(eAction);
	if (!cAction.target)
		return;

	MenuOverlay* overlay = getAncestorOfType<MenuOverlay>();
	if (overlay) {
		overlay->requestDelete();
	}
}


} // namespace ui
} // namespace rack
