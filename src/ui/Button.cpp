#include <ui/Button.hpp>
#include <app.hpp>
#include <event.hpp>


namespace rack {
namespace ui {


Button::Button() {
	box.size.y = BND_WIDGET_HEIGHT;
}

void Button::draw(const DrawArgs& args) {
	BNDwidgetState state = BND_DEFAULT;
	if (APP->event->hoveredWidget == this)
		state = BND_HOVER;
	if (APP->event->draggedWidget == this)
		state = BND_ACTIVE;
	bndToolButton(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
}

void Button::onDragStart(const event::DragStart& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if (quantity)
		quantity->setMax();
}

void Button::onDragEnd(const event::DragEnd& e) {
	if (quantity)
		quantity->setMin();
}

void Button::onDragDrop(const event::DragDrop& e) {
	if (e.origin == this) {
		event::Action eAction;
		onAction(eAction);
	}
}


} // namespace ui
} // namespace rack
