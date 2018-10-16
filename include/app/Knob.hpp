#pragma once
#include "app/common.hpp"
#include "app/ParamWidget.hpp"


namespace rack {


/** Implements vertical dragging behavior for ParamWidgets */
struct Knob : ParamWidget {
	/** Snap to nearest integer while dragging */
	bool snap = false;
	/** Multiplier for mouse movement to adjust knob value */
	float speed = 1.0;
	float dragValue;
	Knob();
	void onDragStart(event::DragStart &e) override;
	void onDragMove(event::DragMove &e) override;
	void onDragEnd(event::DragEnd &e) override;
};


} // namespace rack
