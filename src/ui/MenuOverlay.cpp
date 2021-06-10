#include <ui/MenuOverlay.hpp>


namespace rack {
namespace ui {


MenuOverlay::MenuOverlay() {
	bgColor = nvgRGBA(0, 0, 0, 0);
}


void MenuOverlay::draw(const DrawArgs& args) {
	if (bgColor.a > 0.f) {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, VEC_ARGS(box.size));
		nvgFillColor(args.vg, bgColor);
		nvgFill(args.vg);
	}

	OpaqueWidget::draw(args);
}


void MenuOverlay::step() {
	// Adopt parent's size
	box = parent->box.zeroPos();

	Widget::step();
}


void MenuOverlay::onButton(const ButtonEvent& e) {
	OpaqueWidget::onButton(e);
	if (e.isConsumed() && e.getTarget() != this)
		return;

	if (e.action == GLFW_PRESS) {
		ActionEvent eAction;
		onAction(eAction);
	}

	// Consume all buttons.
	e.consume(this);
}


void MenuOverlay::onHoverKey(const HoverKeyEvent& e) {
	OpaqueWidget::onHoverKey(e);
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ESCAPE) {
		ActionEvent eAction;
		onAction(eAction);
	}

	// Consume all keys.
	// Unfortunately this prevents MIDI computer keyboard from playing while a menu is open, but that might be a good thing for safety.
	e.consume(this);
}


void MenuOverlay::onAction(const ActionEvent& e) {
	requestDelete();
}


} // namespace ui
} // namespace rack
