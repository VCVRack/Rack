#include "app.hpp"


namespace rack {


void ToggleSwitch::onDragStart(EventDragStart &e) {
	// Cycle through values
	// e.g. a range of [0.0, 3.0] would have modes 0, 1, 2, and 3.
	if (value >= maxValue)
		setValue(minValue);
	else
		setValue(value + 1.0);
}


} // namespace rack
