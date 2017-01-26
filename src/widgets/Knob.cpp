#include "rack.hpp"


namespace rack {

#define KNOB_SENSITIVITY 0.001


void Knob::step() {
	index = eucmod((int) roundf(mapf(value, minValue, maxValue, minIndex, maxIndex)), spriteCount);
}

void Knob::onDragStart() {
	guiCursorLock();
}

void Knob::onDragMove(Vec mouseRel) {
	setValue(value - KNOB_SENSITIVITY * (maxValue - minValue) * mouseRel.y);
}

void Knob::onDragEnd() {
	guiCursorUnlock();
}


} // namespace rack
