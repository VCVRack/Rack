#include "ui.hpp"
#include "window.hpp"


namespace rack {

void Tooltip::step() {
	// Follow the mouse
	box.pos = gMousePos;

	// Wrap size to contents
	// box.size = getChildrenBoundingBox().getBottomRight();

	Widget::step();
}


void Tooltip::draw(NVGcontext *vg) {
	bndTooltipBackground(vg, 0.0, 0.0, box.size.x, box.size.y);
	Widget::draw(vg);
}


} // namespace rack
