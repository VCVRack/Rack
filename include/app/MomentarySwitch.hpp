#pragma once
#include "app/common.hpp"


namespace rack {


/** A switch that is turned on when held and turned off when released.
Consider using SVGButton if the switch simply changes the state of your Module when clicked.
*/
struct MomentarySwitch : virtual ParamWidget {
	/** Don't randomize state */
	void randomize() override {}
	void onDragStart(event::DragStart &e) override;
	void onDragEnd(event::DragEnd &e) override;
};


} // namespace rack
