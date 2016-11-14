#include "../5V.hpp"


void Tooltip::step() {
	// Follow the mouse
	box.pos = gMousePos.minus(parent->getAbsolutePos());

	// Wrap size to contents
	// box.size = getChildrenBoundingBox().getBottomRight();
}


void Tooltip::draw(NVGcontext *vg) {
	bndTooltipBackground(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
	Widget::draw(vg);
}
