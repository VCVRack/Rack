#include "ui.hpp"


namespace rack {

void RadioButton::draw(NVGcontext *vg) {
	bndRadioButton(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, value == 0.0 ? state : BND_ACTIVE, -1, label.c_str());
}

void RadioButton::onMouseEnter(EventMouseEnter &e) {
	state = BND_HOVER;
}

void RadioButton::onMouseLeave(EventMouseLeave &e) {
	state = BND_DEFAULT;
}

void RadioButton::onDragDrop(EventDragDrop &e) {
	if (e.origin == this) {
		if (value)
			setValue(0.0);
		else
			setValue(1.0);

		EventAction eAction;
		onAction(eAction);
	}
}


} // namespace rack
