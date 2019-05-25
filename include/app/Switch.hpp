#pragma once
#include <app/common.hpp>
#include <app/ParamWidget.hpp>


namespace rack {
namespace app {


/** A ParamWidget that represents an integer.
Increases by 1 each time it is clicked.
When maxValue is reached, the next click resets to minValue.
In momentary mode, the value is instead set to maxValue when the mouse is held and minValue when released.
*/
struct Switch : ParamWidget {
	/** Return to original position when released */
	bool momentary = false;
	/** Hysteresis state for momentary switch */
	bool momentaryPressed = false;
	bool momentaryReleased = false;

	void step() override;
	void onDoubleClick(const event::DoubleClick &e) override;
	void onDragStart(const event::DragStart &e) override;
	void onDragEnd(const event::DragEnd &e) override;
	void reset() override;
	void randomize() override;
};


} // namespace app
} // namespace rack
