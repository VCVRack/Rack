#include "ui/ScrollWidget.hpp"
#include "app.hpp"
#include "widget/event.hpp"


namespace rack {
namespace ui {


ScrollWidget::ScrollWidget() {
	container = new widget::Widget;
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
	offset = offset.clampSafe(bound);
}

void ScrollWidget::draw(const DrawArgs &args) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));
	Widget::draw(args);
	nvgResetScissor(args.vg);
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

void ScrollWidget::onHover(const widget::HoverEvent &e) {
	widget::Widget::onHover(e);
}

void ScrollWidget::onHoverScroll(const widget::HoverScrollEvent &e) {
	widget::Widget::onHoverScroll(e);
	if (e.getConsumed())
		return;

	// Scroll only if the scrollbars are visible
	if (!(horizontalScrollBar->visible || verticalScrollBar->visible))
		return;

	math::Vec scrollDelta = e.scrollDelta;
	// Flip coordinates if shift is held
	if ((APP->window->getMods() & WINDOW_MOD_MASK) == GLFW_MOD_SHIFT)
		scrollDelta = scrollDelta.flip();

	offset = offset.minus(scrollDelta);
	e.consume(this);
}


} // namespace ui
} // namespace rack
