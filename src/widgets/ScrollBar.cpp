#include "widgets.hpp"
#include "gui.hpp"


namespace rack {


void ScrollBar::draw(NVGcontext *vg) {
	ScrollWidget *scrollWidget = dynamic_cast<ScrollWidget*>(parent);
	assert(scrollWidget);
	Vec containerCorner = scrollWidget->container->getChildrenBoundingBox().getBottomRight();

	float containerSize = (orientation == HORIZONTAL) ? containerCorner.x : containerCorner.y;
	float boxSize = (orientation == HORIZONTAL) ? box.size.x : box.size.y;
	float offset = (orientation == HORIZONTAL) ? scrollWidget->offset.x : scrollWidget->offset.y;
	offset = offset / (containerSize - boxSize);
	float size = boxSize / containerSize;
	size = clampf(size, 0.0, 1.0);
	bndScrollBar(vg, 0.0, 0.0, box.size.x, box.size.y, state, offset, size);
}

void ScrollBar::onDragStart() {
	state = BND_ACTIVE;
	guiCursorLock();
}

void ScrollBar::onDragMove(Vec mouseRel) {
	ScrollWidget *scrollWidget = dynamic_cast<ScrollWidget*>(parent);
	assert(scrollWidget);
	if (orientation == HORIZONTAL)
		scrollWidget->offset.x += mouseRel.x;
	else
		scrollWidget->offset.y += mouseRel.y;
}

void ScrollBar::onDragEnd() {
	state = BND_DEFAULT;
	guiCursorUnlock();
}


} // namespace rack
