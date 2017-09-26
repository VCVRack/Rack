#include "widgets.hpp"
#include "gui.hpp"


namespace rack {

void ScrollBar::step() {
	float boxSize = (orientation == VERTICAL ? box.size.y : box.size.x);
	float maxOffset = containerSize - boxSize;
	containerOffset = clampf(containerOffset, 0.0, maxOffset);
	Widget::step();
}

void ScrollBar::draw(NVGcontext *vg) {
	float boxSize = (orientation == VERTICAL ? box.size.y : box.size.x);
	float maxOffset = containerSize - boxSize;
	float offset = containerOffset / maxOffset;
	float size = boxSize / containerSize;
	size = clampf(size, 0.0, 1.0);
	bndScrollBar(vg, 0.0, 0.0, box.size.x, box.size.y, state, offset, size);
}

void ScrollBar::onDragStart() {
	state = BND_ACTIVE;
	guiCursorLock();
}

void ScrollBar::onDragMove(Vec mouseRel) {
	containerOffset += (orientation == VERTICAL ? mouseRel.y : mouseRel.x);
}

void ScrollBar::onDragEnd() {
	state = BND_DEFAULT;
	guiCursorUnlock();
}


} // namespace rack
