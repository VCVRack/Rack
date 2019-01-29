#include "ui/Button.hpp"


namespace rack {
namespace ui {


Button::Button() {
	box.size.y = BND_WIDGET_HEIGHT;
}

Button::~Button() {
	if (quantity)
		delete quantity;
}

void Button::draw(const widget::DrawContext &ctx) {
	bndToolButton(ctx.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
}

void Button::onEnter(const event::Enter &e) {
	state = BND_HOVER;
	e.consume(this);
}

void Button::onLeave(const event::Leave &e) {
	state = BND_DEFAULT;
}

void Button::onDragStart(const event::DragStart &e) {
	state = BND_ACTIVE;
	if (quantity)
		quantity->setMax();
	e.consume(this);
}

void Button::onDragEnd(const event::DragEnd &e) {
	state = BND_HOVER;
	if (quantity)
		quantity->setMin();
}

void Button::onDragDrop(const event::DragDrop &e) {
	if (e.origin == this) {
		event::Action eAction;
		onAction(eAction);
	}
}


} // namespace ui
} // namespace rack
