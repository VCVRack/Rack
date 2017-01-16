#include "rack.hpp"


namespace rack {

void Label::draw(NVGcontext *vg) {
	bndLabel(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, -1, text.c_str());
}


} // namespace rack
