#include "widgets.hpp"


namespace rack {

void Label::draw(NVGcontext *vg) {
	bndLabel(vg, 0.0, 0.0, box.size.x, box.size.y, -1, text.c_str());
}


} // namespace rack
