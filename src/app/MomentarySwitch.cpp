#include "app/MomentarySwitch.hpp"


namespace rack {


void MomentarySwitch::onDragStart(const event::DragStart &e) {
	if (quantity) {
		quantity->setMax();
	}
}

void MomentarySwitch::onDragEnd(const event::DragEnd &e) {
	if (quantity) {
		quantity->setMin();
	}
}


} // namespace rack
