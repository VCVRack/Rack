#include "app/ToggleSwitch.hpp"


namespace rack {


void ToggleSwitch::onDragStart(event::DragStart &e) {
	// Cycle through values
	// e.g. a range of [0.0, 3.0] would have modes 0, 1, 2, and 3.
	if (quantity) {
		if (quantity->isMax())
			quantity->setMin();
		else
			quantity->moveValue(1.f);
	}
}


} // namespace rack
