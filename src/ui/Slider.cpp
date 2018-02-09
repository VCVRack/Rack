#include "ui.hpp"
#include "window.hpp"


namespace rack {

#define SLIDER_SENSITIVITY 0.001

void Slider::draw(NVGcontext *vg) {
	float progress = rescale(value, minValue, maxValue, 0.0, 1.0);
	bndSlider(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, progress, getText().c_str(), NULL);
}

void Slider::onDragStart(EventDragStart &e) {
	state = BND_ACTIVE;
	windowCursorLock();
}

void Slider::onDragMove(EventDragMove &e) {
	setValue(value + SLIDER_SENSITIVITY * (maxValue - minValue) * e.mouseRel.x);
}

void Slider::onDragEnd(EventDragEnd &e) {
	state = BND_DEFAULT;
	windowCursorUnlock();
	EventAction eAction;
	onAction(eAction);
}

void Slider::onMouseDown(EventMouseDown &e) {
	if (e.button == 1) {
		setValue(defaultValue);
		EventAction eAction;
		onAction(eAction);
	}
	e.consumed = true;
	e.target = this;
}


} // namespace rack
