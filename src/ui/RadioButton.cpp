#include "ui/RadioButton.hpp"


namespace rack {


RadioButton::RadioButton() {
	box.size.y = BND_WIDGET_HEIGHT;
}

RadioButton::~RadioButton() {
	if (quantity)
		delete quantity;
}

void RadioButton::draw(const DrawContext &ctx) {
	std::string label;
	if (quantity)
		label = quantity->getLabel();
	bndRadioButton(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, label.c_str());
}

void RadioButton::onEnter(const event::Enter &e) {
	if (state != BND_ACTIVE)
		state = BND_HOVER;
}

void RadioButton::onLeave(const event::Leave &e) {
	if (state != BND_ACTIVE)
		state = BND_DEFAULT;
}

void RadioButton::onDragDrop(const event::DragDrop &e) {
	if (e.origin == this) {
		if (state == BND_ACTIVE) {
			state = BND_HOVER;
			if (quantity)
				quantity->setMin();
		}
		else {
			state = BND_ACTIVE;
			if (quantity)
				quantity->setMax();
		}

		event::Action eAction;
		onAction(eAction);
	}
}


} // namespace rack
