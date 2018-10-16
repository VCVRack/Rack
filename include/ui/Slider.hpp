#pragma once
#include "ui/common.hpp"


namespace rack {


static const float SLIDER_SENSITIVITY = 0.001f;


struct Slider : OpaqueWidget, QuantityWidget {
	BNDwidgetState state = BND_DEFAULT;

	Slider() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	void draw(NVGcontext *vg) override {
		float progress = rescale(value, minValue, maxValue, 0.0, 1.0);
		bndSlider(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, progress, getText().c_str(), NULL);
	}

	void onDragStart(event::DragStart &e) override {
		state = BND_ACTIVE;
		windowCursorLock();
	}

	void onDragMove(event::DragMove &e) override {
		setValue(value + SLIDER_SENSITIVITY * (maxValue - minValue) * e.mouseDelta.x);
	}

	void onDragEnd(event::DragEnd &e) override {
		state = BND_DEFAULT;
		windowCursorUnlock();
	}

	void onButton(event::Button &e) override {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			setValue(defaultValue);
		}
		e.target = this;
	}
};


} // namespace rack
