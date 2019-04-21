#include "ui/RadioButton.hpp"


namespace rack {
namespace ui {


RadioButton::RadioButton() {
	box.size.y = BND_WIDGET_HEIGHT;
}

void RadioButton::draw(const DrawArgs &args) {
	BNDwidgetState state = this->state;
	std::string label;
	if (quantity) {
		label = quantity->getLabel();
		if (quantity->isMax())
			state = BND_ACTIVE;
	}
	bndRadioButton(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, label.c_str());
}

void RadioButton::onEnter(const event::Enter &e) {
	state = BND_HOVER;
	e.consume(this);
}

void RadioButton::onLeave(const event::Leave &e) {
	state = BND_DEFAULT;
}

void RadioButton::onDragStart(const event::DragStart &e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	e.consume(this);
}

void RadioButton::onDragDrop(const event::DragDrop &e) {
	if (e.origin == this) {
		if (quantity) {
			if (quantity->isMax())
				quantity->setMin();
			else
				quantity->setMax();
		}

		event::Action eAction;
		onAction(eAction);
	}
}


} // namespace ui
} // namespace rack
