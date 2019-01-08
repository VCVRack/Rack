#pragma once
#include "widgets/OpaqueWidget.hpp"
#include "ui/Quantity.hpp"
#include "ui/common.hpp"
#include "app.hpp"


namespace rack {


static const float SLIDER_SENSITIVITY = 0.001f;


struct Slider : OpaqueWidget {
	BNDwidgetState state = BND_DEFAULT;
	Quantity *quantity = NULL;

	Slider() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	~Slider() {
		if (quantity)
			delete quantity;
	}

	void draw(NVGcontext *vg) override {
		float progress = quantity ? quantity->getScaledValue() : 0.f;
		std::string text = quantity ? quantity->getString() : "";
		bndSlider(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, progress, text.c_str(), NULL);
	}

	void onDragStart(const event::DragStart &e) override {
		state = BND_ACTIVE;
		app()->window->cursorLock();
	}

	void onDragMove(const event::DragMove &e) override {
		if (quantity) {
			quantity->moveScaledValue(SLIDER_SENSITIVITY * e.mouseDelta.x);
		}
	}

	void onDragEnd(const event::DragEnd &e) override {
		state = BND_DEFAULT;
		app()->window->cursorUnlock();
	}

	void onButton(const event::Button &e) override {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (quantity)
				quantity->reset();
		}
		e.consume(this);
	}
};


} // namespace rack
