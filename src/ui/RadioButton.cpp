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

void RadioButton::onEnter(const widget::EnterEvent &e) {
	state = BND_HOVER;
	e.consume(this);
}

void RadioButton::onLeave(const widget::LeaveEvent &e) {
	state = BND_DEFAULT;
}

void RadioButton::onDragStart(const widget::DragStartEvent &e) {
	e.consume(this);
}

void RadioButton::onDragDrop(const widget::DragDropEvent &e) {
	if (e.origin == this) {
		if (quantity) {
			if (quantity->isMax())
				quantity->setMin();
			else
				quantity->setMax();
		}

		widget::ActionEvent eAction;
		onAction(eAction);
	}
}


} // namespace ui
} // namespace rack
