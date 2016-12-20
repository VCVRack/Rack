#include "Rack.hpp"


namespace rack {

Screw::Screw() {
	box.size = Vec(15, 15);
	spriteOffset = Vec(-7, -7);
	spriteSize = Vec(29, 29);
	spriteFilename = "res/screw.png";

	std::uniform_int_distribution<> dist(0, 4);
	index = dist(rng);
}


} // namespace rack
