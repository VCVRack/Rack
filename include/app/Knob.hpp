#pragma once
#include <app/common.hpp>
#include <app/ParamWidget.hpp>
#include <context.hpp>


namespace rack {
namespace app {


/** Implements vertical dragging behavior for ParamWidgets */
struct Knob : ParamWidget {
	struct Internal;
	Internal* internal;

	/** Drag horizontally instead of vertically. */
	bool horizontal = false;
	/** Enables per-sample value smoothing while dragging.
	Alternatively, use ParamQuantity::smoothEnabled.
	*/
	bool smooth = true;
	/** Enables value snapping to the nearest integer.
	Alternatively, use ParamQuantity::snapEnabled.
	*/
	bool snap = false;
	/** Multiplier for mouse movement to adjust knob value */
	float speed = 1.f;
	/** Force dragging to linear, e.g. for sliders. */
	bool forceLinear = false;
	/** Angles in radians.
	For drawing and handling the global radial knob setting.
	*/
	float minAngle = -M_PI;
	float maxAngle = M_PI;

	Knob();
	~Knob();
	void initParamQuantity() override;
	void onHover(const HoverEvent& e) override;
	void onButton(const ButtonEvent& e) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
	void onDragMove(const DragMoveEvent& e) override;
	void onDragLeave(const DragLeaveEvent& e) override;
	void onHoverScroll(const HoverScrollEvent& e) override;
	void onLeave(const LeaveEvent& e) override;
	/** Called when user clicks the knob without moving it.
	Useful for handling emulating push-knobs in hardware.
	*/
	void onAction(const ActionEvent& e) override {}
};


} // namespace app
} // namespace rack
