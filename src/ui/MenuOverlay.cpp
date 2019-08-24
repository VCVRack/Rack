#include <ui/MenuOverlay.hpp>


namespace rack {
namespace ui {


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
		e.consume(this);
	}
}


} // namespace ui
} // namespace rack
