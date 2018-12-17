#include "app/MomentarySwitch.hpp"


namespace rack {


void MomentarySwitch::onDragStart(event::DragStart &e) {
	if (quantity) {
		quantity->setMax();
	}
}

void MomentarySwitch::onDragEnd(event::DragEnd &e) {
	if (quantity) {
		quantity->setMin();
	}
}


} // namespace rack
