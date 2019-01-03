#pragma once
#include "app/common.hpp"
#include "app/ParamWidget.hpp"


namespace rack {


/** A switch that is turned on when held and turned off when released.
Consider using SVGButton if the switch simply changes the state of your Module when clicked.
*/
struct MomentarySwitch : virtual ParamWidget {
	void onDragStart(const event::DragStart &e) override;
	void onDragEnd(const event::DragEnd &e) override;
};


} // namespace rack
