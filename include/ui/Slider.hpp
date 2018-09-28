#pragma once

#include "widgets.hpp"
#include "blendish.h"


namespace rack {


static const float SLIDER_SENSITIVITY = 0.001f;


struct Slider : OpaqueWidget, QuantityWidget {
	BNDwidgetState state = BND_DEFAULT;

	Slider() {
		box.size.y = BND_WIDGET_HEIGHT;
	}

	void draw(NVGcontext *vg) override {
		float progress = math::rescale(value, minValue, maxValue, 0.0, 1.0);
		bndSlider(vg, 0.0, 0.0, box.size.x, box.size.y, BND_CORNER_NONE, state, progress, getText().c_str(), NULL);
	}

	void on(event::DragStart &e) override {
		state = BND_ACTIVE;
		windowCursorLock();
	}

	void on(event::DragMove &e) override {
		setValue(value + SLIDER_SENSITIVITY * (maxValue - minValue) * e.mouseDelta.x);
	}

	void on(event::DragEnd &e) override {
		state = BND_DEFAULT;
		windowCursorUnlock();
	}

	void on(event::Button &e) override {
		if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			setValue(defaultValue);
		}
		e.target = this;
	}
};


} // namespace rack
