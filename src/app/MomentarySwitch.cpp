#include "app.hpp"


namespace rack {


void MomentarySwitch::onDragStart(event::DragStart &e) {
	setValue(maxValue);
}

void MomentarySwitch::onDragEnd(event::DragEnd &e) {
	setValue(minValue);
}


} // namespace rack
