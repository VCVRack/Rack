#include "../5V.hpp"


void Label::draw(NVGcontext *vg) {
	bndLabel(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, -1, text.c_str());
}
