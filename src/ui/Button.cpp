#include "ui/Button.hpp"


namespace rack {
namespace ui {


Button::Button() {
	box.size.y = BND_WIDGET_HEIGHT;
}

void Button::draw(const DrawArgs &args) {
	bndToolButton(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
}

void Button::onEnter(const widget::EnterEvent &e) {
	state = BND_HOVER;
	e.consume(this);
}

void Button::onLeave(const widget::LeaveEvent &e) {
	state = BND_DEFAULT;
}

void Button::onDragStart(const widget::DragStartEvent &e) {
	state = BND_ACTIVE;
	if (quantity)
		quantity->setMax();
	e.consume(this);
}

void Button::onDragEnd(const widget::DragEndEvent &e) {
	state = BND_HOVER;
	if (quantity)
		quantity->setMin();
}

void Button::onDragDrop(const widget::DragDropEvent &e) {
	if (e.origin == this) {
		widget::ActionEvent eAction;
		onAction(eAction);
	}
}


} // namespace ui
} // namespace rack
