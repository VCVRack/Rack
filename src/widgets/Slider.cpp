#include "../5V.hpp"


#define SLIDER_SENSITIVITY 0.001

Slider::Slider() {
	box.size.y = BND_WIDGET_HEIGHT;
}

void Slider::draw(NVGcontext *vg) {
	float progress = mapf(value, minValue, maxValue, 0.0, 1.0);
	char quantity[100];
	snprintf(quantity, 100, "%.3g%s", value, unit.c_str());
	bndSlider(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, BND_CORNER_NONE, state, progress, label.c_str(), quantity);
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
