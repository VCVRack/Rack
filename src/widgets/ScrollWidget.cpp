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
	const float arrowSpeed = 50.0;
	if (glfwGetKey(gWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
		offset = offset.minus(Vec(1, 0).mult(arrowSpeed));
	}
	if (glfwGetKey(gWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		offset = offset.minus(Vec(-1, 0).mult(arrowSpeed));
	}
	if (glfwGetKey(gWindow, GLFW_KEY_UP) == GLFW_PRESS) {
		offset = offset.minus(Vec(0, 1).mult(arrowSpeed));
	}
	if (glfwGetKey(gWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
		offset = offset.minus(Vec(0, -1).mult(arrowSpeed));
	}
	return OpaqueWidget::onMouseMove(pos, mouseRel);
}

bool ScrollWidget::onScrollOpaque(Vec scrollRel) {
	offset = offset.minus(scrollRel);
	return true;
}


} // namespace rack
