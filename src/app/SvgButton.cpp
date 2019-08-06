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

void SvgButton::addFrame(std::shared_ptr<Svg> svg) {
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

void SvgButton::onDragStart(const event::DragStart& e) {
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if (frames.size() >= 2) {
		sw->setSvg(frames[1]);
		fb->dirty = true;
	}
}

void SvgButton::onDragEnd(const event::DragEnd& e) {
	if (frames.size() >= 1) {
		sw->setSvg(frames[0]);
		fb->dirty = true;
	}
}

void SvgButton::onDragDrop(const event::DragDrop& e) {
	if (e.origin == this) {
		event::Action eAction;
		onAction(eAction);
	}
}


} // namespace app
} // namespace rack
