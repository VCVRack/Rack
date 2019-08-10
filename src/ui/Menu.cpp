#include <ui/Menu.hpp>

namespace rack {
namespace ui {


Menu::Menu() {
	box.size = math::Vec(0, 0);
}

Menu::~Menu() {
	setChildMenu(NULL);
}

void Menu::setChildMenu(Menu* menu) {
	if (childMenu) {
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
	box.size = math::Vec(0, 0);
	for (widget::Widget* child : children) {
		if (!child->visible)
			continue;
		// Increment height, set position of child
		child->box.pos = math::Vec(0, box.size.y);
		box.size.y += child->box.size.y;
		// Increase width based on maximum width of child
		if (child->box.size.x > box.size.x) {
			box.size.x = child->box.size.x;
		}
	}

	// Set widths of all children to maximum width
	for (widget::Widget* child : children) {
		child->box.size.x = box.size.x;
	}

	// Fit inside parent
	assert(parent);
	box = box.nudge(parent->box.zeroPos());
}

void Menu::draw(const DrawArgs& args) {
	bndMenuBackground(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE);
	Widget::draw(args);
}

void Menu::onHoverScroll(const event::HoverScroll& e) {
	if (parent && !parent->box.isContaining(box))
		box.pos.y += e.scrollDelta.y;
}


} // namespace ui
} // namespace rack
