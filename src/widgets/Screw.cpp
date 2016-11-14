#include "../5V.hpp"


Screw::Screw() {
	box.size = Vec(15, 15);
	spriteOffset = Vec(-7, -7);
	spriteSize = Vec(29, 29);
	spriteFilename = "res/screw.png";
	index = rand() % 5;
}
