#include <ui/MenuOverlay.hpp>


namespace rack {
namespace ui {


void MenuOverlay::draw(const DrawArgs& args) {
	// Possible translucent background
	// nvgRect(args.vg, 0, 0, VEC_ARGS(box.size));
	// nvgFillColor(args.vg, nvgRGBAf(0, 0, 0, 0.25));
	// nvgFill(args.vg);

	OpaqueWidget::draw(args);
}

void MenuOverlay::step() {
	// Adopt parent's size
	box.size = parent->box.size;

	Widget::step();
}

void MenuOverlay::onButton(const event::Button& e) {
	OpaqueWidget::onButton(e);
	if (e.isConsumed() && e.getTarget() != this)
		return;

	if (e.action == GLFW_PRESS) {
		requestDelete();
		e.consume(this);
	}
}

void MenuOverlay::onHoverKey(const event::HoverKey& e) {
	OpaqueWidget::onHoverKey(e);
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ESCAPE) {
		requestDelete();
	}

	// Consume all key presses
	e.consume(this);
}


} // namespace ui
} // namespace rack
