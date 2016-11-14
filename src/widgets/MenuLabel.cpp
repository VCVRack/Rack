#include "../5V.hpp"


void MenuLabel::draw(NVGcontext *vg) {
	bndMenuLabel(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, -1, text.c_str());
}
