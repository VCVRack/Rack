#include "../5V.hpp"


void ChoiceButton::draw(NVGcontext *vg) {
	bndChoiceButton(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, BND_CORNER_NONE, state, -1, text.c_str());
}
