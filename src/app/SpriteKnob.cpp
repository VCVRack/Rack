#include "app.hpp"


namespace rack {

void SpriteKnob::step() {
	index = eucmodi((int) roundf(mapf(value, minValue, maxValue, minIndex, maxIndex)), spriteCount);
}

} // namespace rack
