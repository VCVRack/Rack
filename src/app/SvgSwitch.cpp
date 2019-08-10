#include <app/SvgSwitch.hpp>


namespace rack {
namespace app {


SvgSwitch::SvgSwitch() {
	fb = new widget::FramebufferWidget;
	addChild(fb);

	shadow = new CircularShadow;
	fb->addChild(shadow);
	shadow->box.size = math::Vec();

	sw = new widget::SvgWidget;
	fb->addChild(sw);
}

void SvgSwitch::addFrame(std::shared_ptr<Svg> svg) {
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

void SvgSwitch::onChange(const event::Change& e) {
	if (!frames.empty() && paramQuantity) {
		int index = (int) std::round(paramQuantity->getValue() - paramQuantity->getMinValue());
		index = math::clamp(index, 0, (int) frames.size() - 1);
		sw->setSvg(frames[index]);
		fb->dirty = true;
	}
	ParamWidget::onChange(e);
}


} // namespace app
} // namespace rack
