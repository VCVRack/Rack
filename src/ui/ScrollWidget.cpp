#include "ui.hpp"
#include "window.hpp"


namespace rack {

ScrollWidget::ScrollWidget() {
	container = new Widget();
	addChild(container);

	horizontalScrollBar = new ScrollBar();
	horizontalScrollBar->orientation = ScrollBar::HORIZONTAL;
	horizontalScrollBar->visible = false;
	addChild(horizontalScrollBar);

	verticalScrollBar = new ScrollBar();
	verticalScrollBar->orientation = ScrollBar::VERTICAL;
	verticalScrollBar->visible = false;
	addChild(verticalScrollBar);
}

void ScrollWidget::draw(NVGcontext *vg) {
	nvgScissor(vg, 0, 0, box.size.x, box.size.y);

	Widget::draw(vg);

	nvgResetScissor(vg);
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

	// Update scrollbar offsets and sizes
	Vec viewportSize = container->getChildrenBoundingBox().getBottomRight();
	Vec scrollbarOffset = offset.div(viewportSize.minus(box.size));
	Vec scrollbarSize = box.size.div(viewportSize);

	horizontalScrollBar->offset = scrollbarOffset.x;
	horizontalScrollBar->size = scrollbarSize.x;
	horizontalScrollBar->visible = (0.0 < scrollbarSize.x && scrollbarSize.x < 1.0);
	verticalScrollBar->offset = scrollbarOffset.y;
	verticalScrollBar->size = scrollbarSize.y;
	verticalScrollBar->visible = (0.0 < scrollbarSize.y && scrollbarSize.y < 1.0);

	Widget::step();
}

void ScrollWidget::onMouseMove(EventMouseMove &e) {
	// Scroll with arrow keys
	if (!gFocusedWidget) {
		float arrowSpeed = 30.0;
		if (windowIsShiftPressed() && windowIsModPressed())
			arrowSpeed /= 16.0;
		else if (windowIsShiftPressed())
			arrowSpeed *= 4.0;
		else if (windowIsModPressed())
			arrowSpeed /= 4.0;

		if (windowIsKeyPressed(GLFW_KEY_LEFT)) {
			offset.x -= arrowSpeed;
		}
		if (windowIsKeyPressed(GLFW_KEY_RIGHT)) {
			offset.x += arrowSpeed;
		}
		if (windowIsKeyPressed(GLFW_KEY_UP)) {
			offset.y -= arrowSpeed;
		}
		if (windowIsKeyPressed(GLFW_KEY_DOWN)) {
			offset.y += arrowSpeed;
		}
	}

	Widget::onMouseMove(e);
}

void ScrollWidget::onScroll(EventScroll &e) {
	offset = offset.minus(e.scrollRel);
	e.consumed = true;
}


} // namespace rack
