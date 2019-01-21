#include "app/SVGButton.hpp"


namespace rack {


SVGButton::SVGButton() {
	fb = new FramebufferWidget;
	addChild(fb);

	sw = new SVGWidget;
	fb->addChild(sw);
}

void SVGButton::addFrame(std::shared_ptr<SVG> svg) {
	frames.push_back(svg);
	// If this is our first frame, automatically set SVG and size
	if (!sw->svg) {
		sw->setSVG(svg);
		box.size = sw->box.size;
		fb->box.size = sw->box.size;
	}
}

void SVGButton::onDragStart(const event::DragStart &e) {
	if (frames.size() >= 2) {
		sw->setSVG(frames[1]);
		fb->dirty = true;
	}
}

void SVGButton::onDragEnd(const event::DragEnd &e) {
	if (frames.size() >= 1) {
		sw->setSVG(frames[0]);
		fb->dirty = true;
	}
}

void SVGButton::onDragDrop(const event::DragDrop &e) {
	if (e.origin == this) {
		event::Action eAction;
		onAction(eAction);
	}
}


} // namespace rack
