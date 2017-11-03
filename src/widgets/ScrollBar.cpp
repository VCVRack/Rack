#include "widgets.hpp"
#include "gui.hpp"


namespace rack {


void ScrollBar::draw(NVGcontext *vg) {
	ScrollWidget *scrollWidget = dynamic_cast<ScrollWidget*>(parent);
	assert(scrollWidget);
	Vec viewportCorner = scrollWidget->container->getChildrenBoundingBox().getBottomRight();

	float viewportSize = (orientation == HORIZONTAL) ? viewportCorner.x : viewportCorner.y;
	float containerSize = (orientation == HORIZONTAL) ? scrollWidget->box.size.x : scrollWidget->box.size.y;
	float viewportOffset = (orientation == HORIZONTAL) ? scrollWidget->offset.x : scrollWidget->offset.y;
	float offset = viewportOffset / (viewportSize - containerSize);
	float size = containerSize / viewportSize;
	size = clampf(size, 0.0, 1.0);
	bndScrollBar(vg, 0.0, 0.0, box.size.x, box.size.y, state, offset, size);
}

void ScrollBar::onDragStart(EventDragStart &e) {
	state = BND_ACTIVE;
	guiCursorLock();
}

void ScrollBar::onDragMove(EventDragMove &e) {
	ScrollWidget *scrollWidget = dynamic_cast<ScrollWidget*>(parent);
	assert(scrollWidget);
	if (orientation == HORIZONTAL)
		scrollWidget->offset.x += e.mouseRel.x;
	else
		scrollWidget->offset.y += e.mouseRel.y;
}

void ScrollBar::onDragEnd(EventDragEnd &e) {
	state = BND_DEFAULT;
	guiCursorUnlock();
}


} // namespace rack
