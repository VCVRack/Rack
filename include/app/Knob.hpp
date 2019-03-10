#pragma once
#include "app/common.hpp"
#include "app/ParamWidget.hpp"
#include "app.hpp"


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

	void onHover(const widget::HoverEvent &e) override;
	void onButton(const widget::ButtonEvent &e) override;
	void onDragStart(const widget::DragStartEvent &e) override;
	void onDragEnd(const widget::DragEndEvent &e) override;
	void onDragMove(const widget::DragMoveEvent &e) override;
	void reset() override;
	void randomize() override;
};


} // namespace app
} // namespace rack
