#include "app.hpp"


namespace rack {


void MomentarySwitch::on(event::DragStart &e) {
	setValue(maxValue);
}

void MomentarySwitch::on(event::DragEnd &e) {
	setValue(minValue);
}


} // namespace rack
