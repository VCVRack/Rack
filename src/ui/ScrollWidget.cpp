#include <ui/ScrollWidget.hpp>
#include <context.hpp>


namespace rack {
namespace ui {


// Internal not currently used


ScrollWidget::ScrollWidget() {
	container = new widget::Widget;
	addChild(container);

	horizontalScrollBar = new ScrollBar;
	horizontalScrollBar->vertical = false;
	horizontalScrollBar->hide();
	addChild(horizontalScrollBar);

	verticalScrollBar = new ScrollBar;
	verticalScrollBar->vertical = true;
	verticalScrollBar->hide();
	addChild(verticalScrollBar);
}


void ScrollWidget::scrollTo(math::Rect r) {
	math::Rect bound = math::Rect::fromMinMax(r.getBottomRight().minus(box.size), r.pos);
	offset = offset.clampSafe(bound);
}


void ScrollWidget::draw(const DrawArgs& args) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));
	Widget::draw(args);
	nvgResetScissor(args.vg);
}


void ScrollWidget::step() {
	Widget::step();

	// Clamp scroll offset
	containerBox = container->getChildrenBoundingBox();
	math::Rect offsetBounds = containerBox;
	offsetBounds.size = offsetBounds.size.minus(box.size);
	offset = offset.clamp(offsetBounds);

	// Update the container's position from the offset
	container->box.pos = offset.neg().round();

	// Make scrollbars visible only if there is a positive range to scroll.
	horizontalScrollBar->setVisible(offsetBounds.size.x > 0.f);
	verticalScrollBar->setVisible(offsetBounds.size.y > 0.f);

	// Reposition and resize scroll bars
	math::Vec inner = box.size.minus(math::Vec(verticalScrollBar->box.size.x, horizontalScrollBar->box.size.y));
	horizontalScrollBar->box.pos.y = inner.y;
	verticalScrollBar->box.pos.x = inner.x;
	horizontalScrollBar->box.size.x = verticalScrollBar->isVisible() ? inner.x : box.size.x;
	verticalScrollBar->box.size.y = horizontalScrollBar->isVisible() ? inner.y : box.size.y;
}


void ScrollWidget::onButton(const event::Button& e) {
	OpaqueWidget::onButton(e);
	if (e.isConsumed())
		return;

	// Consume right button only if the scrollbars are visible
	if (!(horizontalScrollBar->visible || verticalScrollBar->visible))
		return;

	if (e.button == GLFW_MOUSE_BUTTON_MIDDLE) {
		e.consume(this);
	}
}


void ScrollWidget::onDragStart(const event::DragStart& e) {
	if (e.button == GLFW_MOUSE_BUTTON_LEFT || e.button == GLFW_MOUSE_BUTTON_MIDDLE) {
		e.consume(this);
	}
}


void ScrollWidget::onDragMove(const event::DragMove& e) {
	// Scroll only if the scrollbars are visible
	if (!(horizontalScrollBar->visible || verticalScrollBar->visible))
		return;

	offset = offset.minus(e.mouseDelta);
}


void ScrollWidget::onHoverScroll(const event::HoverScroll& e) {
	OpaqueWidget::onHoverScroll(e);
	if (e.isConsumed())
		return;

	// Scroll only if the scrollbars are visible
	if (!(horizontalScrollBar->visible || verticalScrollBar->visible))
		return;

	math::Vec scrollDelta = e.scrollDelta;
	// Flip coordinates if shift is held
	// Mac (or GLFW?) already does this for us.
#if !defined ARCH_MAC
	if ((APP->window->getMods() & RACK_MOD_MASK) == GLFW_MOD_SHIFT)
		scrollDelta = scrollDelta.flip();
#endif

	offset = offset.minus(scrollDelta);
	e.consume(this);
}


void ScrollWidget::onHoverKey(const event::HoverKey& e) {
	OpaqueWidget::onHoverKey(e);
	if (e.isConsumed())
		return;

	if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
		if (e.key == GLFW_KEY_PAGE_UP && (e.mods & RACK_MOD_MASK) == 0) {
			offset.y -= box.size.y * 0.5;
			e.consume(this);
		}
		if (e.key == GLFW_KEY_PAGE_UP && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			offset.x -= box.size.x * 0.5;
			e.consume(this);
		}
		if (e.key == GLFW_KEY_PAGE_DOWN && (e.mods & RACK_MOD_MASK) == 0) {
			offset.y += box.size.y * 0.5;
			e.consume(this);
		}
		if (e.key == GLFW_KEY_PAGE_DOWN && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			offset.x += box.size.x * 0.5;
			e.consume(this);
		}
		if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == 0) {
			math::Rect containerBox = container->getChildrenBoundingBox();
			offset.y = containerBox.getTop();
			e.consume(this);
		}
		if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			math::Rect containerBox = container->getChildrenBoundingBox();
			offset.x = containerBox.getLeft();
			e.consume(this);
		}
		if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == 0) {
			math::Rect containerBox = container->getChildrenBoundingBox();
			offset.y = containerBox.getBottom();
			e.consume(this);
		}
		if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			math::Rect containerBox = container->getChildrenBoundingBox();
			offset.x = containerBox.getRight();
			e.consume(this);
		}
	}
}


} // namespace ui
} // namespace rack
