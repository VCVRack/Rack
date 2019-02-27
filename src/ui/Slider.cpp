#include "ui/Slider.hpp"


namespace rack {
namespace ui {


static const float SENSITIVITY = 0.001f;


Slider::Slider() {
	box.size.y = BND_WIDGET_HEIGHT;
}

Slider::~Slider() {
	if (quantity)
		delete quantity;
}

void Slider::draw(const DrawArgs &args) {
	float progress = quantity ? quantity->getScaledValue() : 0.f;
	std::string text = quantity ? quantity->getString() : "";
	bndSlider(args.vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, progress, text.c_str(), NULL);
}

void Slider::onDragStart(const widget::DragStartEvent &e) {
	state = BND_ACTIVE;
	APP->window->cursorLock();
	e.consume(this);
}

void Slider::onDragMove(const widget::DragMoveEvent &e) {
	if (quantity) {
		quantity->moveScaledValue(SENSITIVITY * e.mouseDelta.x);
	}
}

void Slider::onDragEnd(const widget::DragEndEvent &e) {
	state = BND_DEFAULT;
	APP->window->cursorUnlock();
}

void Slider::onDoubleClick(const widget::DoubleClickEvent &e) {
	if (quantity)
		quantity->reset();
}


} // namespace ui
} // namespace rack
