#include "app.hpp"


namespace rack {

void SpriteKnob::step() {
	index = eucmod((int) roundf(mapf(value, minValue, maxValue, minIndex, maxIndex)), spriteCount);
}

} // namespace rack
