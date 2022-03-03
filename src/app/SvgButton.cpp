#include <app/SvgButton.hpp>


namespace rack {
namespace app {


SvgButton::SvgButton() {
	fb = new widget::FramebufferWidget;
	addChild(fb);

	shadow = new CircularShadow;
	fb->addChild(shadow);
	shadow->box.size = math::Vec();

	sw = new widget::SvgWidget;
	fb->addChild(sw);
}


void SvgButton::onButton(const ButtonEvent& e) {
	OpaqueWidget::onButton(e);

	// Dispatch ActionEvent on left click
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
		ActionEvent eAction;
		onAction(eAction);
	}
}


void SvgButton::addFrame(std::shared_ptr<window::Svg> svg) {
	frames.push_back(svg);
	// If this is our first frame, automatically set SVG and size
	if (!sw->svg) {
		sw->setSvg(svg);
		box.size = sw->box.size;
		fb->box.size = sw->box.size;
		// Move shadow downward by 10%
		shadow->box.size = sw->box.size;
		shadow->box.pos = math::Vec(0, sw->box.size.y * 0.10);
	}
}


void SvgButton::onDragStart(const DragStartEvent& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if (frames.size() >= 2) {
		sw->setSvg(frames[1]);
		fb->dirty = true;
	}
}


void SvgButton::onDragEnd(const DragEndEvent& e) {
	if (frames.size() >= 1) {
		sw->setSvg(frames[0]);
		fb->dirty = true;
	}
}


void SvgButton::onDragDrop(const DragDropEvent& e) {
	// Don't dispatch ActionEvent on DragDrop because it's already called on mouse down.
	// if (e.origin == this) {
	// 	ActionEvent eAction;
	// 	onAction(eAction);
	// }
}


} // namespace app
} // namespace rack
