#include "ui.hpp"


namespace rack {



void Label::draw(NVGcontext *vg) {
	float x = 0.0;
	if (align == RIGHT_ALIGN) {
		x = box.size.x - bndLabelWidth(vg, -1, text.c_str());
	}
	else if (align == CENTER_ALIGN) {
		x = (box.size.x - bndLabelWidth(vg, -1, text.c_str())) / 2.0;
	}

	bndLabel(vg, x, 0.0, box.size.x, box.size.y, -1, text.c_str());
}


} // namespace rack
