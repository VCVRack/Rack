#include "ValleyWidgets.hpp"

void SVGStepSlider::step() {
	if (dirty) {
		// Interpolate handle position
		handle->box.pos = Vec(rescale(floorf(value), minValue, maxValue, minHandlePos.x, maxHandlePos.x),
                              rescale(floorf(value), minValue, maxValue, minHandlePos.y, maxHandlePos.y));
	}
	FramebufferWidget::step();
}
