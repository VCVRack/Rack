#include "app.hpp"


namespace rack {


void MomentarySwitch::onDragStart(EventDragStart &e) {
	setValue(maxValue);
	EventAction eAction;
	onAction(eAction);
}

void MomentarySwitch::onDragEnd(EventDragEnd &e) {
	setValue(minValue);
}


} // namespace rack
