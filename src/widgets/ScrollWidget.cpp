#include "Rack.hpp"


namespace rack {

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

void ScrollWidget::step() {
	Vec b = Vec(box.size.x - vScrollBar->box.size.x, box.size.y - hScrollBar->box.size.y);

	hScrollBar->box.pos.y = b.y;
	hScrollBar->box.size.x = b.x;

	vScrollBar->box.pos.x = b.x;
	vScrollBar->box.size.y = b.y;

	Widget::step();
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

Widget *ScrollWidget::onScroll(Vec pos, Vec scrollRel) {
	Widget *w = Widget::onScroll(pos, scrollRel);
	if (w) return w;

	hScrollBar->move(scrollRel.x);
	vScrollBar->move(scrollRel.y);
	return this;
}


} // namespace rack
