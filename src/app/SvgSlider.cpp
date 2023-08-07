#include <app/SvgSlider.hpp>


namespace rack {
namespace app {


SvgSlider::SvgSlider() {
	fb = new widget::FramebufferWidget;
	addChild(fb);

	background = new widget::SvgWidget;
	fb->addChild(background);

	handle = new widget::SvgWidget;
	fb->addChild(handle);

	speed = 2.0;
}


void SvgSlider::setBackgroundSvg(std::shared_ptr<window::Svg> svg) {
	if (svg == background->svg)
		return;

	background->setSvg(svg);
	box.size = background->box.size;
	fb->box.size = background->box.size;
	fb->setDirty();
}


void SvgSlider::setHandleSvg(std::shared_ptr<window::Svg> svg) {
	if (svg == handle->svg)
		return;

	handle->setSvg(svg);
	handle->box.pos = maxHandlePos;
	fb->setDirty();
}


void SvgSlider::setHandlePos(math::Vec minHandlePos, math::Vec maxHandlePos) {
	this->minHandlePos = minHandlePos;
	this->maxHandlePos = maxHandlePos;

	// Set handle pos to maximum by default
	handle->box.pos = maxHandlePos;
}


void SvgSlider::setHandlePosCentered(math::Vec minHandlePosCentered, math::Vec maxHandlePosCentered) {
	setHandlePos(
		minHandlePosCentered.minus(handle->box.size.div(2)),
		maxHandlePosCentered.minus(handle->box.size.div(2))
	);
}


void SvgSlider::onChange(const ChangeEvent& e) {
	// Default position is max value
	float v = 1.f;
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		v = pq->getScaledValue();
	}

	// Interpolate handle position
	handle->box.pos = minHandlePos.crossfade(maxHandlePos, v);
	fb->setDirty();

	ParamWidget::onChange(e);
}


} // namespace app
} // namespace rack
