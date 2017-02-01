#include "widgets.hpp"


namespace rack {

void RadioButton::draw(NVGcontext *vg) {
	bndRadioButton(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, BND_CORNER_NONE, value == 0.0 ? state : BND_ACTIVE, -1, label.c_str());
}

void RadioButton::onMouseEnter() {
	state = BND_HOVER;
}

void RadioButton::onMouseLeave() {
	state = BND_DEFAULT;
}

void RadioButton::onDragDrop(Widget *origin) {
	if (origin == this) {
		if (value == 0.0)
			value = 1.0;
		else
			value = 0.0;
	}
}


} // namespace rack
