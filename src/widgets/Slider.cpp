#include "rack.hpp"


namespace rack {

#define SLIDER_SENSITIVITY 0.001

void Slider::draw(NVGcontext *vg) {
	float progress = mapf(value, minValue, maxValue, 0.0, 1.0);
	bndSlider(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, BND_CORNER_NONE, state, progress, getText().c_str(), NULL);
}

void Slider::onDragStart() {
	state = BND_ACTIVE;
	guiCursorLock();
}

void Slider::onDragMove(Vec mouseRel) {
	setValue(value + SLIDER_SENSITIVITY * (maxValue - minValue) * mouseRel.x);
}

void Slider::onDragEnd() {
	state = BND_DEFAULT;
	guiCursorUnlock();
}


} // namespace rack
