#include "app/SVGSwitch.hpp"


namespace rack {


SVGSwitch::SVGSwitch() {
	fb = new FramebufferWidget;
	addChild(fb);

	sw = new SVGWidget;
	fb->addChild(sw);
}

void SVGSwitch::addFrame(std::shared_ptr<SVG> svg) {
	frames.push_back(svg);
	// If this is our first frame, automatically set SVG and size
	if (!sw->svg) {
		sw->setSVG(svg);
		box.size = sw->box.size;
		fb->box.size = sw->box.size;
	}
}

void SVGSwitch::onChange(const event::Change &e) {
	if (!frames.empty() && paramQuantity) {
		int index = (int) std::round(paramQuantity->getValue());
		index = math::clamp(index, 0, (int) frames.size() - 1);
		sw->setSVG(frames[index]);
		fb->dirty = true;
	}
	ParamWidget::onChange(e);
}


} // namespace rack
