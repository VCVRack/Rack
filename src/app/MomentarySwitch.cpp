#include "app/MomentarySwitch.hpp"


namespace rack {


void MomentarySwitch::onDragStart(const event::DragStart &e) {
	if (paramQuantity) {
		paramQuantity->setMax();
	}
}

void MomentarySwitch::onDragEnd(const event::DragEnd &e) {
	if (paramQuantity) {
		paramQuantity->setMin();
	}
}


} // namespace rack
