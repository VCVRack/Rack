#include "ui/ScrollWidget.hpp"
#include "context.hpp"
#include "event.hpp"


namespace rack {


ScrollWidget::ScrollWidget() {
	container = new Widget;
	addChild(container);

	horizontalScrollBar = new ScrollBar;
	horizontalScrollBar->orientation = ScrollBar::HORIZONTAL;
	horizontalScrollBar->visible = false;
	addChild(horizontalScrollBar);

	verticalScrollBar = new ScrollBar;
	verticalScrollBar->orientation = ScrollBar::VERTICAL;
	verticalScrollBar->visible = false;
	addChild(verticalScrollBar);
}

void ScrollWidget::scrollTo(math::Rect r) {
	math::Rect bound = math::Rect::fromMinMax(r.getBottomRight().minus(box.size), r.pos);
	offset = offset.clampBetween(bound);
}

void ScrollWidget::draw(NVGcontext *vg) {
	nvgScissor(vg, 0, 0, box.size.x, box.size.y);
	Widget::draw(vg);
	nvgResetScissor(vg);
}

void ScrollWidget::step() {
	Widget::step();

	// Clamp scroll offset
	math::Vec containerCorner = container->getChildrenBoundingBox().getBottomRight();
	math::Rect containerBox = math::Rect(math::Vec(0, 0), containerCorner.minus(box.size));
	offset = offset.clamp(containerBox);
	// Lock offset to top/left if no scrollbar will display
	if (containerBox.size.x < 0.0)
		offset.x = 0.0;
	if (containerBox.size.y < 0.0)
		offset.y = 0.0;

	// Update the container's positions from the offset
	container->box.pos = offset.neg().round();

	// Update scrollbar offsets and sizes
	math::Vec viewportSize = container->getChildrenBoundingBox().getBottomRight();
	math::Vec scrollbarOffset = offset.div(viewportSize.minus(box.size));
	math::Vec scrollbarSize = box.size.div(viewportSize);

	horizontalScrollBar->visible = (0.0 < scrollbarSize.x && scrollbarSize.x < 1.0);
	verticalScrollBar->visible = (0.0 < scrollbarSize.y && scrollbarSize.y < 1.0);
	horizontalScrollBar->offset = scrollbarOffset.x;
	verticalScrollBar->offset = scrollbarOffset.y;
	horizontalScrollBar->size = scrollbarSize.x;
	verticalScrollBar->size = scrollbarSize.y;

	// Resize scroll bars
	math::Vec inner = math::Vec(box.size.x - verticalScrollBar->box.size.x, box.size.y - horizontalScrollBar->box.size.y);
	horizontalScrollBar->box.pos.y = inner.y;
	verticalScrollBar->box.pos.x = inner.x;
	horizontalScrollBar->box.size.x = verticalScrollBar->visible ? inner.x : box.size.x;
	verticalScrollBar->box.size.y = horizontalScrollBar->visible ? inner.y : box.size.y;
}

void ScrollWidget::onHover(const event::Hover &e) {
	// Scroll with arrow keys
	if (!context()->event->selectedWidget) {
		float arrowSpeed = 30.0;
		if (context()->window->isShiftPressed() && context()->window->isModPressed())
			arrowSpeed /= 16.0;
		else if (context()->window->isShiftPressed())
			arrowSpeed *= 4.0;
		else if (context()->window->isModPressed())
			arrowSpeed /= 4.0;

		if (glfwGetKey(context()->window->win, GLFW_KEY_LEFT) == GLFW_PRESS) {
			offset.x -= arrowSpeed;
		}
		if (glfwGetKey(context()->window->win, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			offset.x += arrowSpeed;
		}
		if (glfwGetKey(context()->window->win, GLFW_KEY_UP) == GLFW_PRESS) {
			offset.y -= arrowSpeed;
		}
		if (glfwGetKey(context()->window->win, GLFW_KEY_DOWN) == GLFW_PRESS) {
			offset.y += arrowSpeed;
		}
	}

	OpaqueWidget::onHover(e);
}

void ScrollWidget::onHoverScroll(const event::HoverScroll &e) {
	offset = offset.minus(e.scrollDelta);
	e.consume(this);
}


} // namespace rack
