#include "../5V.hpp"


ScrollWidget::ScrollWidget() {
	container = new Widget();
	addChild(container);

	hScrollBar = new ScrollBar();
	hScrollBar->orientation = ScrollBar::HORIZONTAL;
	addChild(hScrollBar);

	vScrollBar = new ScrollBar();
	vScrollBar->orientation = ScrollBar::VERTICAL;
	addChild(vScrollBar);
}

void ScrollWidget::draw(NVGcontext *vg) {
	// Update the scrollbar sizes
	Vec c = container->getChildrenBoundingBox().getBottomRight();
	hScrollBar->containerSize = c.x;
	vScrollBar->containerSize = c.y;

	// Update the container's positions from the scrollbar offsets
	container->box.pos = Vec(-hScrollBar->containerOffset, -vScrollBar->containerOffset).round();

	Widget::draw(vg);
}

void ScrollWidget::onResize() {
	Vec b = Vec(box.size.x - vScrollBar->box.size.x, box.size.y - hScrollBar->box.size.y);

	hScrollBar->box.pos.y = b.y;
	hScrollBar->box.size.x = b.x;

	vScrollBar->box.pos.x = b.x;
	vScrollBar->box.size.y = b.y;
}

void ScrollWidget::onScroll(Vec scrollRel) {
	hScrollBar->move(scrollRel.x);
	vScrollBar->move(scrollRel.y);
}
