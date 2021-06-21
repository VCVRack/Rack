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


void SvgSlider::setBackgroundSvg(std::shared_ptr<Svg> svg) {
	background->setSvg(svg);
	box.size = background->box.size;
	fb->box.size = background->box.size;
	fb->setDirty();
}


void SvgSlider::setHandleSvg(std::shared_ptr<Svg> svg) {
	handle->setSvg(svg);
	handle->box.pos = minHandlePos;
	fb->setDirty();
}


void SvgSlider::onChange(const ChangeEvent& e) {
	engine::ParamQuantity* pq = getParamQuantity();
	if (pq) {
		// Interpolate handle position
		float v = math::rescale(pq->getSmoothValue(), pq->getMinValue(), pq->getMaxValue(), 0.f, 1.f);
		handle->box.pos = minHandlePos.crossfade(maxHandlePos, v);
		fb->setDirty();
	}
	ParamWidget::onChange(e);
}


void SvgSlider::setHandlePos(math::Vec minHandlePos, math::Vec maxHandlePos) {
	this->minHandlePos = minHandlePos;
	this->maxHandlePos = maxHandlePos;
}


void SvgSlider::setHandlePosCentered(math::Vec minHandlePosCentered, math::Vec maxHandlePosCentered) {
	this->minHandlePos = minHandlePosCentered.minus(handle->box.size.div(2));
	this->maxHandlePos = maxHandlePosCentered.minus(handle->box.size.div(2));
}


} // namespace app
} // namespace rack
