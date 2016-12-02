#include "Rack.hpp"


namespace rack {

ScrollBar::ScrollBar() {
	box.size.x = BND_SCROLLBAR_WIDTH;
	box.size.y = BND_SCROLLBAR_HEIGHT;
}

void ScrollBar::draw(NVGcontext *vg) {
	float boxSize = (orientation == VERTICAL ? box.size.y : box.size.x);
	float maxOffset = containerSize - boxSize;
	float offset = containerOffset / maxOffset;
	float size = boxSize / containerSize;
	size = clampf(size, 0.0, 1.0);
	bndScrollBar(vg, box.pos.x, box.pos.y, box.size.x, box.size.y, state, offset, size);
}

void ScrollBar::move(float delta) {
	float boxSize = (orientation == VERTICAL ? box.size.y : box.size.x);
	float maxOffset = containerSize - boxSize;
	containerOffset += delta;
	containerOffset = clampf(containerOffset, 0.0, maxOffset);
}

void ScrollBar::onDragStart() {
	state = BND_ACTIVE;
	guiCursorLock();
}

void ScrollBar::onDragMove(Vec mouseRel) {
	float delta = (orientation == VERTICAL ? mouseRel.y : mouseRel.x);
	move(delta);
}

void ScrollBar::onDragEnd() {
	state = BND_DEFAULT;
	guiCursorUnlock();
}


} // namespace rack
