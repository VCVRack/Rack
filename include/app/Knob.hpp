#pragma once
#include "app/common.hpp"
#include "app/ParamWidget.hpp"


namespace rack {


static const float KNOB_SENSITIVITY = 0.0015f;


/** Implements vertical dragging behavior for ParamWidgets */
struct Knob : ParamWidget {
	/** Multiplier for mouse movement to adjust knob value */
	float speed = 1.0;

	void onDragStart(event::DragStart &e) override {
		windowCursorLock();
	}

	void onDragEnd(event::DragEnd &e) override {
		windowCursorUnlock();
	}

	void onDragMove(event::DragMove &e) override {
		if (quantity) {
			float range;
			if (quantity->isBounded()) {
				range = quantity->getRange();
			}
			else {
				// Continuous encoders scale as if their limits are +/-1
				range = 2.f;
			}
			float delta = KNOB_SENSITIVITY * -e.mouseDelta.y * speed * range;

			// Drag slower if Mod is held
			if (windowIsModPressed())
				delta /= 16.f;
			quantity->moveValue(delta);
		}
	}
};


} // namespace rack
