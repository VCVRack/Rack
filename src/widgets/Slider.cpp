#include "widgets.hpp"
#include "gui.hpp"


namespace rack {

#define SLIDER_SENSITIVITY 0.001

void Slider::draw(NVGcontext *vg) {
	float progress = rescalef(value, minValue, maxValue, 0.0, 1.0);
	bndSlider(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, progress, getText().c_str(), NULL);
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
	onAction();
}

void Slider::onMouseDownOpaque(int button) {
	if (button == 1) {
		setValue(defaultValue);
		onAction();
	}
}


} // namespace rack
