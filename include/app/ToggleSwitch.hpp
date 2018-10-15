#pragma once
#include "common.hpp"


namespace rack {


/** A switch that cycles through each mechanical position */
struct ToggleSwitch : virtual ParamWidget {
	void onDragStart(event::DragStart &e) override;
};


} // namespace rack
