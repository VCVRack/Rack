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
	struct Internal;
	Internal* internal;

	/** Return to original position when released */
	bool momentary = false;

	Switch();
	~Switch();
	void initParamQuantity() override;
	void step() override;
	void onDoubleClick(const DoubleClickEvent& e) override;
	void onDragStart(const DragStartEvent& e) override;
	void onDragEnd(const DragEndEvent& e) override;
};


} // namespace app
} // namespace rack
