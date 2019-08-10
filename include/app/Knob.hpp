#pragma once
#include <app/common.hpp>
#include <app/ParamWidget.hpp>
#include <app.hpp>


namespace rack {
namespace app {


/** Implements vertical dragging behavior for ParamWidgets */
struct Knob : ParamWidget {
	/** Multiplier for mouse movement to adjust knob value */
	float speed = 1.0;
	float oldValue = 0.f;
	bool smooth = true;
	/** Enable snapping at integer values */
	bool snap = false;
	float snapValue = NAN;
	/** Drag horizontally instead of vertically */
	bool horizontal = false;

	void onHover(const event::Hover& e) override;
	void onButton(const event::Button& e) override;
	void onDragStart(const event::DragStart& e) override;
	void onDragEnd(const event::DragEnd& e) override;
	void onDragMove(const event::DragMove& e) override;
	void reset() override;
	void randomize() override;
};


} // namespace app
} // namespace rack
