#pragma once
#include <app/common.hpp>
#include <app/ParamWidget.hpp>
#include <app.hpp>


namespace rack {
namespace app {


/** Implements vertical dragging behavior for ParamWidgets */
struct Knob : ParamWidget {
	struct Internal;
	Internal* internal;

	/** Multiplier for mouse movement to adjust knob value */
	float speed = 1.0;
	/** Drag horizontally instead of vertically. */
	bool horizontal = false;
	/** Enables per-sample value smoothing while dragging. */
	bool smooth = true;
	/** DEPRECATED. Use `ParamQuantity::snapEnabled`. */
	bool snap = false;

	Knob();
	~Knob();
	void initParamQuantity() override;
	void onHover(const event::Hover& e) override;
	void onButton(const event::Button& e) override;
	void onDragStart(const event::DragStart& e) override;
	void onDragEnd(const event::DragEnd& e) override;
	void onDragMove(const event::DragMove& e) override;
};


} // namespace app
} // namespace rack
