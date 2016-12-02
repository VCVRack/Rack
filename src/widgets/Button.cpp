#include "Rack.hpp"


namespace rack {

Button::Button() {
	box.size.y = BND_WIDGET_HEIGHT;
}

void Button::draw(NVGcontext *vg) {
	bndToolButton(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
}

void Button::onMouseEnter() {
	state = BND_HOVER;
}

void Button::onMouseLeave() {
	state = BND_DEFAULT;
}

void Button::onDragDrop(Widget *origin) {
	if (origin == this) {
		onAction();
	}
}


} // namespace rack
