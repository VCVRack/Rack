#include "ui/Slider.hpp"


namespace rack {


static const float SENSITIVITY = 0.001f;


Slider::Slider() {
	box.size.y = BND_WIDGET_HEIGHT;
}

Slider::~Slider() {
	if (quantity)
		delete quantity;
}

void Slider::draw(NVGcontext *vg) {
	float progress = quantity ? quantity->getScaledValue() : 0.f;
	std::string text = quantity ? quantity->getString() : "";
	bndSlider(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, progress, text.c_str(), NULL);
}

void Slider::onDragStart(const event::DragStart &e) {
	state = BND_ACTIVE;
	app()->window->cursorLock();
}

void Slider::onDragMove(const event::DragMove &e) {
	if (quantity) {
		quantity->moveScaledValue(SENSITIVITY * e.mouseDelta.x);
	}
}

void Slider::onDragEnd(const event::DragEnd &e) {
	state = BND_DEFAULT;
	app()->window->cursorUnlock();
}

void Slider::onButton(const event::Button &e) {
	if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (quantity)
			quantity->reset();
	}
	e.consume(this);
}


} // namespace rack
