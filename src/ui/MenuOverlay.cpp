#include <ui/MenuOverlay.hpp>


namespace rack {
namespace ui {


void MenuOverlay::draw(const DrawArgs& args) {
	// Translucent background
	// nvgBeginPath(args.vg);
	// nvgRect(args.vg, 0, 0, VEC_ARGS(box.size));
	// nvgFillColor(args.vg, nvgRGBAf(0, 0, 0, 0.33));
	// nvgFill(args.vg);

	OpaqueWidget::draw(args);
}


void MenuOverlay::step() {
	// Adopt parent's size
	box = parent->box.zeroPos();

	Widget::step();
}


void MenuOverlay::onButton(const event::Button& e) {
	OpaqueWidget::onButton(e);
	if (e.isConsumed() && e.getTarget() != this)
		return;

	if (e.action == GLFW_PRESS) {
		event::Action eAction;
		onAction(eAction);
	}

	// Consume all buttons.
	e.consume(this);
}


void MenuOverlay::onHoverKey(const event::HoverKey& e) {
	OpaqueWidget::onHoverKey(e);
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ESCAPE) {
		event::Action eAction;
		onAction(eAction);
	}

	// Consume all keys.
	// Unfortunately this prevents MIDI computer keyboard from playing while a menu is open, but that might be a good thing for safety.
	e.consume(this);
}


void MenuOverlay::onAction(const event::Action& e) {
	requestDelete();
}


} // namespace ui
} // namespace rack
