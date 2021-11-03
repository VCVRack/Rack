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


SvgSwitch::~SvgSwitch() {
}


void SvgSwitch::addFrame(std::shared_ptr<window::Svg> svg) {
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


void SvgSwitch::onDragStart(const DragStartEvent& e) {
	Switch::onDragStart(e);
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	// Set down frame if latch
	if (latch) {
		if (frames.size() >= 2) {
			sw->setSvg(frames[1]);
			fb->setDirty();
		}
	}
}


void SvgSwitch::onDragEnd(const DragEndEvent& e) {
	Switch::onDragEnd(e);
	if (e.button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	// Set up frame if latch
	if (latch) {
		if (frames.size() >= 1) {
			sw->setSvg(frames[0]);
			fb->setDirty();
		}
	}
}


void SvgSwitch::onChange(const ChangeEvent& e) {
	if (!latch) {
		engine::ParamQuantity* pq = getParamQuantity();
		if (!frames.empty() && pq) {
			int index = (int) std::round(pq->getValue() - pq->getMinValue());
			index = math::clamp(index, 0, (int) frames.size() - 1);
			sw->setSvg(frames[index]);
			fb->setDirty();
		}
	}
	ParamWidget::onChange(e);
}


} // namespace app
} // namespace rack
