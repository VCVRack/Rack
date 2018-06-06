#include "ui.hpp"


namespace rack {

void Button::draw(NVGcontext *vg) {
	bndToolButton(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
	Widget::draw(vg);
}

void Button::onMouseEnter(EventMouseEnter &e) {
	state = BND_HOVER;
}

void Button::onMouseLeave(EventMouseLeave &e) {
	state = BND_DEFAULT;
}

void Button::onDragStart(EventDragStart &e) {
	state = BND_ACTIVE;
}

void Button::onDragEnd(EventDragEnd &e) {
	state = BND_HOVER;
}

void Button::onDragDrop(EventDragDrop &e) {
	if (e.origin == this) {
		EventAction eAction;
		onAction(eAction);
	}
}


} // namespace rack
