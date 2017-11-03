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

	// Scroll with arrow keys
	if (!gFocusedWidget) {
		float arrowSpeed = 30.0;
		if (guiIsShiftPressed() && guiIsModPressed())
			arrowSpeed /= 16.0;
		else if (guiIsShiftPressed())
			arrowSpeed *= 4.0;
		else if (guiIsModPressed())
			arrowSpeed /= 4.0;

		if (glfwGetKey(gWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
			offset.x -= arrowSpeed;
		}
		if (glfwGetKey(gWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			offset.x += arrowSpeed;
		}
		if (glfwGetKey(gWindow, GLFW_KEY_UP) == GLFW_PRESS) {
			offset.y -= arrowSpeed;
		}
		if (glfwGetKey(gWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
			offset.y += arrowSpeed;
		}
	}
	Widget::step();
}

void ScrollWidget::onScroll(EventScroll &e) {
	offset = offset.minus(e.scrollRel);
	e.consumed = true;
}


} // namespace rack
