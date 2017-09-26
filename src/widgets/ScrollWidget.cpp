#include "widgets.hpp"
#include "gui.hpp"


namespace rack {

ScrollWidget::ScrollWidget() {
	container = new Widget();
	addChild(container);

	horizontalScrollBar = new ScrollBar();
	horizontalScrollBar->orientation = ScrollBar::HORIZONTAL;
	addChild(horizontalScrollBar);

	verticalScrollBar = new ScrollBar();
	verticalScrollBar->orientation = ScrollBar::VERTICAL;
	addChild(verticalScrollBar);
}

void ScrollWidget::step() {
	// Clamp scroll offset
	Vec containerCorner = container->getChildrenBoundingBox().getBottomRight();
	offset = offset.clamp(Rect(Vec(0, 0), containerCorner.minus(box.size)));

	// Resize scroll bars
	Vec inner = Vec(box.size.x - verticalScrollBar->box.size.x, box.size.y - horizontalScrollBar->box.size.y);
	horizontalScrollBar->box.pos.y = inner.y;
	horizontalScrollBar->box.size.x = inner.x;
	verticalScrollBar->box.pos.x = inner.x;
	verticalScrollBar->box.size.y = inner.y;

	// Update the container's positions from the offset
	container->box.pos = offset.neg().round();

	Widget::step();
}

Widget *ScrollWidget::onMouseMove(Vec pos, Vec mouseRel) {
	Widget *w = Widget::onMouseMove(pos, mouseRel);
	if (w) return w;

	if (glfwGetMouseButton(gWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
		offset = offset.minus(mouseRel);
		return this;
	}
	return NULL;
}

bool ScrollWidget::onScrollOpaque(Vec scrollRel) {
	offset = offset.minus(scrollRel);
	return true;
}


} // namespace rack
