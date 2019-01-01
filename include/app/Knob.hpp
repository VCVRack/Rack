#pragma once
#include "app/common.hpp"
#include "app/ParamWidget.hpp"
#include "context.hpp"


namespace rack {


static const float KNOB_SENSITIVITY = 0.0015f;


/** Implements vertical dragging behavior for ParamWidgets */
struct Knob : ParamWidget {
	/** Multiplier for mouse movement to adjust knob value */
	float speed = 1.0;

	void onButton(event::Button &e) override;
	void onDragStart(event::DragStart &e) override;
	void onDragEnd(event::DragEnd &e) override;
	void onDragMove(event::DragMove &e) override;
};


} // namespace rack
